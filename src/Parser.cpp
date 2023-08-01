#include "Parser.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <utility>
#include <iterator>
#include <unordered_set>
#include <functional>
#include <tuple>

#include "Diagnostic.hpp"
#include "Diagnostics/ParsingDiagnostics.hpp"
#include "Measured.hpp"
#include "Token.hpp"
#include "Nodes/All.hpp"
#include "Compilation.hpp"
#include "FileBuffer.hpp"
#include "Scope.hpp"
#include "Assert.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdent.hpp"
#include "Name.hpp"
#include "Ident.hpp"

namespace Ace
{
    enum class RefParsingKind
    {
        Allow,
        Disallow,
    };

    enum class Modifier
    {
        Public,
        Extern,
        Self,
    };

    enum class FunctionOrOpNameKind
    {
        Function,
        Op,
    };

    struct FunctionOrOpNameToken
    {
        FunctionOrOpNameKind Kind{};
        std::shared_ptr<const Token> Value{};
    };

    static auto IsCompoundAssignmentOp(
        const TokenKind tokenKind
    ) -> bool
    {
        switch (tokenKind)
        {
            case TokenKind::PlusEquals:
            case TokenKind::MinusEquals:
            case TokenKind::AsteriskEquals:
            case TokenKind::SlashEquals:
            case TokenKind::PercentEquals:
            case TokenKind::LessThanLessThanEquals:
            case TokenKind::GreaterThanGreaterThanEquals:
            case TokenKind::CaretEquals:
            case TokenKind::VerticalBarEquals:
            case TokenKind::AmpersandEquals:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsUserPrefixOp(const TokenKind tokenKind) -> bool
    {
        switch (tokenKind)
        {
            case TokenKind::Plus:
            case TokenKind::Minus:
            case TokenKind::Tilde:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsPrefixOp(const TokenKind tokenKind) -> bool
    {
        if (IsUserPrefixOp(tokenKind))
        {
            return true;
        }

        switch (tokenKind)
        {
            case TokenKind::Exclamation:
            case TokenKind::BoxKeyword:
            case TokenKind::UnboxKeyword:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsPostfixOp(const TokenKind tokenKind) -> bool
    {
        switch (tokenKind)
        {
            case TokenKind::OpenParen:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsUserBinaryOp(const TokenKind tokenKind) -> bool
    {
        switch (tokenKind)
        {
            case TokenKind::Asterisk:
            case TokenKind::Slash:
            case TokenKind::Percent:
            case TokenKind::Plus:
            case TokenKind::Minus:
            case TokenKind::LessThan:
            case TokenKind::GreaterThan:
            case TokenKind::LessThanEquals:
            case TokenKind::GreaterThanEquals:
            case TokenKind::LessThanLessThan:
            case TokenKind::GreaterThanGreaterThan:
            case TokenKind::EqualsEquals:
            case TokenKind::ExclamationEquals:
            case TokenKind::Caret:
            case TokenKind::VerticalBar:
            case TokenKind::Ampersand:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsBinaryOp(const TokenKind tokenKind) -> bool
    {
        if (IsUserBinaryOp(tokenKind))
        {
            return true;
        }

        switch (tokenKind)
        {
            case TokenKind::VerticalBarVerticalBar:
            case TokenKind::AmpersandAmpersand:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsUserOp(const TokenKind tokenKind) -> bool
    {
        return
            IsUserPrefixOp(tokenKind) ||
            IsUserBinaryOp(tokenKind);
    }

    static constexpr size_t MaxBinaryOpPrecedence = 9;
    static auto GetBinaryOpPrecedence(
        const TokenKind op
    ) -> size_t
    {
        if (
            (op == TokenKind::Asterisk) ||
            (op == TokenKind::Slash) ||
            (op == TokenKind::Percent)
            )
        {
            return MaxBinaryOpPrecedence;
        }

        if (
            (op == TokenKind::Plus) ||
            (op == TokenKind::Minus)
            )
        {
            return 8;
        }

        if (
            (op == TokenKind::LessThanLessThan) ||
            (op == TokenKind::GreaterThanGreaterThan)
            )
        {
            return 7;
        }

        if (
            (op == TokenKind::LessThan) ||
            (op == TokenKind::LessThanEquals) ||
            (op == TokenKind::GreaterThan) ||
            (op == TokenKind::GreaterThanEquals)
            )
        {
            return 6;
        }

        if (
            (op == TokenKind::EqualsEquals) ||
            (op == TokenKind::ExclamationEquals)
            )
        {
            return 5;
        }

        if (op == TokenKind::Ampersand)
        {
            return 4;
        }

        if (op == TokenKind::Caret)
        {
            return 3;
        }

        if (op == TokenKind::VerticalBar)
        {
            return 2;
        }

        if (op == TokenKind::AmpersandAmpersand)
        {
            return 1;
        }

        if (op == TokenKind::VerticalBarVerticalBar)
        {
            return 0;
        }

        ACE_UNREACHABLE();
    };

    static auto CreateCollapsedPrefixExpr(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr,
        const Op& op
    ) -> std::shared_ptr<const IExprNode>
    {
        switch (op.TokenKind)
        {
            case TokenKind::Exclamation:
            {
                return std::make_shared<const LogicalNegationExprNode>(
                    srcLocation,
                    expr
                );
            }

            case TokenKind::BoxKeyword:
            {
                return std::make_shared<const BoxExprNode>(
                    srcLocation,
                    expr
                );
            }

            case TokenKind::UnboxKeyword:
            {
                return std::make_shared<const UnboxExprNode>(
                    srcLocation,
                    expr
                );
            }

            default:
            {
                return std::make_shared<const UserUnaryExprNode>(
                    srcLocation,
                    expr, 
                    op
                );
            }
        }
    }

    static auto CreateCollapsedBinaryExpr(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& lhsExpr,
        const std::shared_ptr<const IExprNode>& rhsExpr,
        const Op& op
    ) -> std::shared_ptr<const IExprNode>
    {
        switch (op.TokenKind)
        {
            case TokenKind::AmpersandAmpersand:
            {
                return std::make_shared<const AndExprNode>(
                    srcLocation,
                    lhsExpr,
                    rhsExpr
                );
            }

            case TokenKind::VerticalBarVerticalBar:
            {
                return std::make_shared<const OrExprNode>(
                    srcLocation,
                    lhsExpr,
                    rhsExpr
                );
            }
             
            default:
            {
                return std::make_shared<const UserBinaryExprNode>(
                    srcLocation,
                    lhsExpr,
                    rhsExpr,
                    op
                );
            }
        }
    }

    static auto CreateSrcLocationRange(
        const std::shared_ptr<const Token>& firstToken,
        const std::shared_ptr<const Token>& lastToken
    ) -> SrcLocation
    {
        return
        {
            firstToken->SrcLocation,
             lastToken->SrcLocation,
        };
    }

    static auto CreateSrcLocationRange(
        const std::shared_ptr<const INode>& firstToken,
        const std::shared_ptr<const INode>& lastToken
    ) -> SrcLocation
    {
        return
        {
            firstToken->GetSrcLocation(),
             lastToken->GetSrcLocation(),
        };
    }

    static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const AttributeNode>>
    {
        return {};
    }

    static auto GetOpFunctionName(
        const std::shared_ptr<const Token>& opToken,
        const size_t paramCount
    ) -> Expected<const char*>
    {
        DiagnosticBag diagnostics{};

        switch (opToken->Kind)
        {
            case TokenKind::Asterisk:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Multiplication,
                    diagnostics,
                };
            }

            case TokenKind::Slash:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Division,
                    diagnostics,
                };
            }

            case TokenKind::Percent:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Remainder,
                    diagnostics,
                };
            }

            case TokenKind::Plus:
            {
                if (paramCount == 1)
                {
                    return
                    {
                        SpecialIdent::Op::UnaryPlus,
                        diagnostics,
                    };
                }

                if (paramCount == 2)
                {
                    return
                    {
                        SpecialIdent::Op::Addition,
                        diagnostics,
                    };
                }

                return diagnostics.Add(
                    CreateUnexpectedUnaryOrBinaryOpParamCountError(opToken)
                );
            }

            case TokenKind::Minus:
            {
                if (paramCount == 1)
                {
                    return
                    {
                        SpecialIdent::Op::UnaryNegation,
                        diagnostics,
                    };
                }

                if (paramCount == 2)
                {
                    return
                    {
                        SpecialIdent::Op::Subtraction,
                        diagnostics,
                    };
                }

                return diagnostics.Add(
                    CreateUnexpectedUnaryOrBinaryOpParamCountError(opToken)
                );
            }

            case TokenKind::LessThan:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::LessThan,
                    diagnostics,
                };
            }

            case TokenKind::GreaterThan:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::GreaterThan,
                    diagnostics,
                };
            }

            case TokenKind::LessThanEquals:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::LessThanEquals,
                    diagnostics,
                };
            }

            case TokenKind::GreaterThanEquals:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::GreaterThanEquals,
                    diagnostics,
                };
            }

            case TokenKind::GreaterThanGreaterThan:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::RightShift,
                    diagnostics,
                };
            }
            
            case TokenKind::LessThanLessThan:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::LeftShift,
                    diagnostics,
                };
            }

            case TokenKind::EqualsEquals:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Equals,
                    diagnostics,
                };
            }

            case TokenKind::ExclamationEquals:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::NotEquals,
                    diagnostics,
                };
            }

            case TokenKind::Caret:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }
    
                return
                {
                    SpecialIdent::Op::XOR,
                    diagnostics,
                };
            }

            case TokenKind::VerticalBar:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::OR,
                    diagnostics,
                };
            }

            case TokenKind::Ampersand:
            {
                if (paramCount != 2)
                {
                    return diagnostics.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::AND,
                    diagnostics,
                };
            }
            
            case TokenKind::Tilde:
            {
                if (paramCount != 1)
                {
                    return diagnostics.Add(
                        CreateUnexpectedUnaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::OneComplement,
                    diagnostics,
                };
            }
            
            case TokenKind::Ident:
            {
                if (opToken->String == SpecialIdent::Copy)
                {
                    if (paramCount != 2)
                    {
                        return diagnostics.Add(
                            CreateUnexpectedBinaryOpParamCountError(opToken)
                        );
                    }

                    return
                    {
                        SpecialIdent::Op::Copy,
                        diagnostics,
                    };
                }
                
                if (opToken->String == SpecialIdent::Drop)
                {
                    if (paramCount != 1)
                    {
                        return diagnostics.Add(
                            CreateUnexpectedUnaryOpParamCountError(opToken)
                        );
                    }

                    return
                    {
                        SpecialIdent::Op::Drop,
                        diagnostics,
                    };
                }

                return diagnostics.Add(
                    CreateUnknownIdentOpError(opToken)
                );
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    static auto CreateFunctionOrOpName(
        const FunctionOrOpNameToken& nameToken,
        const size_t paramCount,
        const AccessModifier accessModifier,
        const bool hasSelfParam
    ) -> Expected<Ident>
    {
        DiagnosticBag diagnostics{};

        if (nameToken.Kind == FunctionOrOpNameKind::Function)
        {
            ACE_ASSERT(nameToken.Value->Kind == TokenKind::Ident);
            return 
            {
                Ident
                {
                    nameToken.Value->SrcLocation,
                    nameToken.Value->String,
                },
                diagnostics,
            };
        }

        const auto expName = GetOpFunctionName(nameToken.Value, paramCount);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        if (accessModifier != AccessModifier::Public)
        {
            return diagnostics.Add(
                CreateOpMustBePublicError(nameToken.Value)
            );
        }

        if (hasSelfParam)
        {
            return diagnostics.Add(
                CreateInstanceOpError(nameToken.Value)
            );
        }

        return Ident
        {
            nameToken.Value->SrcLocation,
            expName.Unwrap(),
        };
    }

    static auto CreateSelfParam(
        const std::optional<std::shared_ptr<const Token>>& optSelfToken,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName
    ) -> std::optional<std::shared_ptr<const SelfParamVarNode>>
    {
        if (!optSelfToken.has_value())
        {
            return std::nullopt;
        }

        return std::make_shared<const SelfParamVarNode>(
            optSelfToken.value()->SrcLocation,
            scope,
            selfTypeName
        );
    }

    static auto GetLiteralKind(
        const std::shared_ptr<const Token>& token
    ) -> Expected<LiteralKind>
    {
        DiagnosticBag diagnostics{};

        switch (token->Kind)
        {
            case TokenKind::Int8:
            {
                return Expected{ LiteralKind::Int8, diagnostics };
            }

            case TokenKind::Int16:
            {
                return Expected{ LiteralKind::Int16, diagnostics };
            }

            case TokenKind::Int32:
            {
                return Expected{ LiteralKind::Int32, diagnostics };
            }

            case TokenKind::Int64:
            {
                return Expected{ LiteralKind::Int64, diagnostics };
            }

            case TokenKind::UInt8:
            {
                return Expected{ LiteralKind::UInt8, diagnostics };
            }

            case TokenKind::UInt16:
            {
                return Expected{ LiteralKind::UInt16, diagnostics };
            }

            case TokenKind::UInt32:
            {
                return Expected{ LiteralKind::UInt32, diagnostics };
            }

            case TokenKind::UInt64:
            {
                return Expected{ LiteralKind::UInt64, diagnostics };
            }

            case TokenKind::Int:
            {
                return Expected{ LiteralKind::Int, diagnostics };
            }

            case TokenKind::Float32:
            {
                return Expected{ LiteralKind::Float32, diagnostics };
            }

            case TokenKind::Float64:
            {
                return Expected{ LiteralKind::Float64, diagnostics };
            }

            case TokenKind::String:
            {
                return Expected{ LiteralKind::String, diagnostics };
            }

            case TokenKind::TrueKeyword:
            {
                return Expected{ LiteralKind::True, diagnostics };
            }

            case TokenKind::FalseKeyword:
            {
                return Expected{ LiteralKind::False, diagnostics };
            }

            default:
            {
                return diagnostics.Add(CreateUnexpectedTokenExpectedLiteralError(
                    token
                ));
            };
        }
    }

    static auto GetModifier(
        const std::shared_ptr<const Token>& token
    ) -> Expected<Modifier>
    {
        DiagnosticBag diagnostics{};

        switch (token->Kind)
        {
            case TokenKind::PublicKeyword:
            {
                return { Modifier::Public, diagnostics };
            }

            case TokenKind::ExternKeyword:
            {
                return { Modifier::Extern, diagnostics };
            }

            case TokenKind::Ident:
            {
                if (token->String == SpecialIdent::Self)
                {
                    return { Modifier::Self, diagnostics };
                }
            }

            default:
            {
                return diagnostics.Add(CreateUnknownModifierError(
                    token
                ));
            }
        }
    }

    enum class DiscardKind
    {
        Inclusive,
        Exclusive,
    };

    struct DiscardInfo
    {
        DiscardKind Kind{};
        TokenKind TokenKind{};
    };

    class Parser
    {
    public:
        Parser(
            const FileBuffer* const fileBuffer,
            const std::vector<std::shared_ptr<const Token>>& tokens
        ) : m_FileBuffer{ fileBuffer }
        {
            const auto firstLineIt = begin(fileBuffer->GetLines());
            const auto  lastLineIt =   end(fileBuffer->GetLines()) - 1;

            const auto& firstLine = *firstLineIt;
            const auto&  lastLine = * lastLineIt;

            const SrcLocation leadingSrcLocation
            {
                fileBuffer,
                begin(firstLine),
                begin(firstLine) + 1,
            };
            const SrcLocation trailingSrcLocation
            {
                fileBuffer,
                end(lastLine) - 1,
                end(lastLine),
            };

            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSrcLocation,
                TokenKind::Ident,
                fileBuffer->GetCompilation()->Package.Name
            ));
            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSrcLocation,
                TokenKind::Colon
            ));
            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSrcLocation,
                TokenKind::ModuleKeyword
            ));
            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSrcLocation,
                TokenKind::OpenBrace
            ));

            std::transform(
                begin(tokens),
                end  (tokens),
                back_inserter(m_Tokens),
                [](const std::shared_ptr<const Token>& token)
                {
                    return token;
                }
            );

            m_Tokens.insert(
                end(m_Tokens) - 1,
                std::make_shared<const Token>(
                    trailingSrcLocation,
                    TokenKind::CloseBrace
                )
            );

               m_Iterator = begin(m_Tokens);
            m_EndIterator = end  (m_Tokens) - 1;
        }
        ~Parser() = default;

        auto GetFileBuffer() const -> const FileBuffer*
        {
            return m_FileBuffer;
        }
        auto GetNestLevel() const -> size_t
        {
            return m_NestLevel;
        }
        auto GetSrcLocation() const -> const SrcLocation&
        {
            return Peek()->SrcLocation;
        }
        auto GetLastSrcLocation() const -> const SrcLocation&
        {
            return m_LastSrcLocation;
        }
        auto IsEnd() const -> bool
        {
            return m_Iterator == m_EndIterator;
        }
        auto Peek(const size_t distance = 0) const -> const std::shared_ptr<const Token>&
        {
            return *(m_Iterator + distance);
        }

        auto Eat() -> const std::shared_ptr<const Token>&
        {
            m_LastSrcLocation = GetSrcLocation();

            m_Iterator++;
            ACE_ASSERT(m_Iterator <= m_EndIterator);
            UpdateNestLevel();

            return *(m_Iterator - 1);
        }

        auto DiscardUntil(
            const DiscardKind kind,
            const TokenKind tokenKind
        ) -> void
        {
            DiscardUntil(kind, std::vector{ tokenKind });
        }
        auto DiscardUntil(
            const DiscardKind kind,
            const std::vector<TokenKind>& tokenKinds
        ) -> void
        {
            const auto beginNestLevel = GetNestLevel();

            while (!IsEnd())
            {
                if (GetNestLevel() != beginNestLevel)
                {
                    Eat();
                    continue;
                }

                const auto matchingTokenKindIt = std::find_if(
                    begin(tokenKinds),
                    end  (tokenKinds),
                    [&](const TokenKind tokenKind)
                    {
                        return Peek() == tokenKind;
                    }
                );
                if (matchingTokenKindIt == end(tokenKinds))
                {
                    Eat();
                    continue;
                }

                if (kind == DiscardKind::Inclusive)
                {
                    Eat();
                }

                break;
            }
        }
        
    private:
        auto UpdateNestLevel() -> void
        {
            if (IsEnd())
            {
                return;
            }

            if (Peek() == TokenKind::CloseBrace)
            {
                ssize_t signedNestLevel = static_cast<ssize_t>(m_NestLevel);
                signedNestLevel--;
                if (signedNestLevel >= 0)
                {
                    m_NestLevel--;
                }
            }

            if (Peek() == TokenKind::OpenBrace)
            {
                m_NestLevel++;
            }
        }

        const FileBuffer* m_FileBuffer{};
        std::vector<std::shared_ptr<const Token>> m_Tokens{};
        std::vector<std::shared_ptr<const Token>>::const_iterator m_Iterator{};
        std::vector<std::shared_ptr<const Token>>::const_iterator m_EndIterator{};
        size_t m_NestLevel{};
        SrcLocation m_LastSrcLocation{};
    };

    static auto IsKeywordExprBegin(
        const Parser& parser
    ) -> bool
    {
        switch (parser.Peek()->Kind)
        {
            case TokenKind::CastKeyword:
            case TokenKind::AddressOfKeyword:
            case TokenKind::SizeOfKeyword:
            case TokenKind::DerefAsKeyword:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsExprExprBegin(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == TokenKind::OpenParen;
    }

    static auto IsStructConstructionExprBegin(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == SpecialIdent::New;
    }

    static auto IsLiteralExprBegin(
        const Parser& parser
    ) -> bool
    {
        switch (parser.Peek()->Kind)
        {
            case TokenKind::Int8:
            case TokenKind::Int16:
            case TokenKind::Int32:
            case TokenKind::Int64:
            case TokenKind::UInt8:
            case TokenKind::UInt16:
            case TokenKind::UInt32:
            case TokenKind::UInt64:
            case TokenKind::Int:
            case TokenKind::Float32:
            case TokenKind::Float64:
            case TokenKind::String:
            case TokenKind::TrueKeyword:
            case TokenKind::FalseKeyword:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsSymbolLiteralExprBegin(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == TokenKind::Ident;
    }

    static auto IsKeywordStmtBegin(
        const Parser& parser
    ) -> bool
    {
        switch (parser.Peek()->Kind)
        {
            case TokenKind::IfKeyword:
            case TokenKind::WhileKeyword:
            case TokenKind::ReturnKeyword:
            case TokenKind::ExitKeyword:
            case TokenKind::AssertKeyword:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsBlockStmtBegin(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == TokenKind::OpenBrace;
    }

    static auto IsModuleBegin(
        const Parser& parser
    ) -> bool
    {
        if (parser.Peek(0) != TokenKind::Ident)
        {
            return false;
        }

        size_t i = 1;
        while (parser.Peek(i) == TokenKind::ColonColon)
        {
            i++;

            if (parser.Peek(i) != TokenKind::Ident)
            {
                return false;
            }

            i++;
        }

        return
            (parser.Peek(i + 0) == TokenKind::Colon) &&
            (parser.Peek(i + 1) == TokenKind::ModuleKeyword);
    }

    static auto GetItemTemplateCloseBracketIndex(
        const Parser& parser
    ) -> std::optional<size_t>
    {
        if (parser.Peek(0) != TokenKind::Ident)
        {
            return std::nullopt;
        }

        if (parser.Peek(1) != TokenKind::OpenBracket)
        {
            return std::nullopt;
        }

        size_t i = 2;
        while (
            !parser.IsEnd() &&
            (parser.Peek(i) != TokenKind::CloseBracket)
            )
        {
            if (i != 2)
            {
                if (parser.Peek(i) != TokenKind::Comma)
                {
                    return std::nullopt;
                }

                i++;
            }

            if (parser.Peek(i) != TokenKind::Ident)
            {
                return std::nullopt;
            }

            i++;
        }

        return (parser.Peek(i) == TokenKind::CloseBracket) ?
            i : std::optional<size_t>{};
    }

    static auto IsStructBegin(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::Ident) &&
            (parser.Peek(1) == TokenKind::Colon) &&
            (parser.Peek(2) == TokenKind::StructKeyword);
    }

    static auto IsTypeBegin(
        const Parser& parser
    ) -> bool
    {
        return IsStructBegin(parser);
    }

    static auto IsStructTemplateBegin(
        const Parser& parser
    ) -> bool
    {
        const auto optCloseBracketIndex = GetItemTemplateCloseBracketIndex(
            parser
        );
        if (!optCloseBracketIndex.has_value())
        {
            return false;
        }

        auto i = optCloseBracketIndex.value() + 1;

        if (parser.Peek(i) != TokenKind::Colon)
        {
            return false;
        }

        i++;

        return parser.Peek(i) == TokenKind::StructKeyword;
    }

    static auto IsTypeTemplateBegin(
        const Parser& parser
    ) -> bool
    {
        return IsStructTemplateBegin(parser);
    }

    static auto IsFunctionBegin(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::OpKeyword) ||
            (
                (parser.Peek(0) == TokenKind::Ident) &&
                (parser.Peek(1) == TokenKind::OpenParen)
            );
    }

    static auto IsFunctionTemplateBegin(
        const Parser& parser
    ) -> bool
    {
        const auto optCloseBracketIndex = GetItemTemplateCloseBracketIndex(
            parser
        );
        if (!optCloseBracketIndex.has_value())
        {
            return false;
        }

        const auto i = optCloseBracketIndex.value() + 1;
        return parser.Peek(i) == TokenKind::OpenParen;
    }

    static auto IsImplBegin(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::ImplKeyword) &&
            (parser.Peek(1) != TokenKind::OpenBracket);
    }

    static auto IsTemplatedImplBegin(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::ImplKeyword) &&
            (parser.Peek(1) == TokenKind::OpenBracket);
    }

    static auto IsVarBegin(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::Ident) &&
            (parser.Peek(1) == TokenKind::Colon) &&
            (parser.Peek(2) != TokenKind::StructKeyword) &&
            (parser.Peek(2) != TokenKind::ModuleKeyword);
    }

    static auto ParseExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>;

    static auto ParseOptionalTemplateArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>;

    static auto ParseStructConstructionExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructConstructionExprNode>>;

    static auto ParseStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IStmtNode>>;

    static auto ParseName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<Ident>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::Ident)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Ident
            ));
        }

        const auto& nameToken = parser.Eat();

        return Expected
        {
            Ident
            {
                nameToken->SrcLocation,
                nameToken->String,
            },
            diagnostics,
        };
    }

    static auto ParseNestedName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<Ident>>
    {
        DiagnosticBag diagnostics{};

        std::vector<Ident> nestedName{};

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        nestedName.push_back(std::move(expName.Unwrap()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto expName = ParseName(parser, scope);
            diagnostics.Add(expName);
            if (!expName)
            {
                return diagnostics;
            }

            nestedName.push_back(std::move(expName.Unwrap()));
        }

        return Expected{ std::move(nestedName), diagnostics };
    }

    static auto ParseSymbolNameSection(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolNameSection>
    {
        DiagnosticBag diagnostics{};

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        const auto expTemplateArgs = ParseOptionalTemplateArgs(
            parser,
            scope
        );
        diagnostics.Add(expTemplateArgs);
        if (!expTemplateArgs)
        {
            return diagnostics;
        }

        return Expected
        {
            SymbolNameSection
            {
                expName.Unwrap(),
                expTemplateArgs.Unwrap(),
            },
            diagnostics,
        };
    }

    static auto ParseSymbolName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolName>
    {
        DiagnosticBag diagnostics{};

        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (parser.Peek() == TokenKind::ColonColon)
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            parser.Eat();
        }
        
        std::vector<SymbolNameSection> sections{};

        const auto expSection = ParseSymbolNameSection(parser, scope);
        diagnostics.Add(expSection);
        if (!expSection)
        {
            return diagnostics;
        }

        sections.push_back(std::move(expSection.Unwrap()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto expSection = ParseSymbolNameSection(parser, scope);
            diagnostics.Add(expSection);
            if (!expSection)
            {
                return diagnostics;
            }

            sections.push_back(std::move(expSection.Unwrap()));
        }

        return Expected
        {
            SymbolName
            {
                sections,
                resolutionScope,
            },
            diagnostics,
        };
    }

    static auto ParseTypeName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const RefParsingKind refParsingKind
    ) -> Expected<TypeName>
    {
        DiagnosticBag diagnostics{};

        std::vector<TypeNameModifier> modifiers{};

        if (refParsingKind == RefParsingKind::Allow)
        {
            if (parser.Peek() == TokenKind::Ampersand)
            {
                modifiers.push_back(TypeNameModifier::Ref);
                parser.Eat();
            }
        }

        while (true)
        {
            if (parser.Peek() == TokenKind::Asterisk)
            {
                modifiers.push_back(TypeNameModifier::StrongPtr);
                parser.Eat();
                continue;
            }

            if (parser.Peek() == TokenKind::Tilde)
            {
                modifiers.push_back(TypeNameModifier::WeakPtr);
                parser.Eat();
                continue;
            }

            break;
        }

        const auto expSymbolName = ParseSymbolName(parser, scope);
        diagnostics.Add(expSymbolName);
        if (!expSymbolName)
        {
            return diagnostics;
        }

        return Expected
        {
            TypeName
            {
                expSymbolName.Unwrap(),
                modifiers,
            },
            diagnostics,
        };
    }

    static auto ParseTemplateParamNames(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<Ident>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        parser.Eat();

        std::vector<Ident> names{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBracket)
            )
        {
            if (!names.empty())
            {
                if (parser.Peek() == TokenKind::Comma)
                {
                    parser.Eat();
                }
                else
                {
                    diagnostics.Add(CreateMissingTokenError(
                        parser.GetLastSrcLocation(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expName = ParseName(parser, scope);
            diagnostics.Add(expName);
            if (expName)
            {
                names.push_back(std::move(expName.Unwrap()));
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseBracket }
            );
        }

        if (parser.Peek() == TokenKind::CloseBracket)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBracket
            ));
        }

        if (names.empty())
        {
            return diagnostics.Add(CreateEmptyTemplateParamsError(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() }
            ));
        }

        return Expected{ names, diagnostics };
    }

    static auto ParseImplTemplateParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const ImplTemplateParamNode>>>
    {
        DiagnosticBag diagnostics{};

        const auto expNames = ParseTemplateParamNames(parser, scope);
        diagnostics.Add(expNames);
        if (!expNames)
        {
            return diagnostics;
        }
        
        std::vector<std::shared_ptr<const ImplTemplateParamNode>> params{};
        std::transform(
            begin(expNames.Unwrap()),
            end  (expNames.Unwrap()),
            back_inserter(params),
            [&](const Ident& name)
            {
                return std::make_shared<const ImplTemplateParamNode>(
                    name.SrcLocation,
                    scope,
                    name
                );
            }
        );

        return Expected{ std::move(params), diagnostics };
    }

    static auto ParseTemplateParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
    {
        DiagnosticBag diagnostics{};

        const auto expNames = ParseTemplateParamNames(parser, scope);
        diagnostics.Add(expNames);
        if (!expNames)
        {
            return diagnostics;
        }
        
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> params{};
        std::transform(
            begin(expNames.Unwrap()),
            end  (expNames.Unwrap()),
            back_inserter(params),
            [&](const Ident& name)
            {
                return std::make_shared<const NormalTemplateParamNode>(
                    name.SrcLocation,
                    scope,
                    name
                );
            }
        );

        return Expected{ std::move(params), diagnostics };
    }

    static auto ParseOptionalTemplateParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
    {
        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return Expected
            {
                std::vector<std::shared_ptr<const NormalTemplateParamNode>>{},
                DiagnosticBag{},
            };
        }

        return ParseTemplateParams(parser, scope);
    }

    static auto ParseTemplateArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        parser.Eat();

        std::vector<SymbolName> args{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBracket)
            )
        {
            if (!args.empty())
            {
                if (parser.Peek() == TokenKind::Comma)
                {
                    parser.Eat();
                }
                else
                {
                    diagnostics.Add(CreateMissingTokenError(
                        parser.GetLastSrcLocation(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expArg = ParseSymbolName(parser, scope);
            diagnostics.Add(expArg);
            if (expArg)
            {
                args.push_back(std::move(expArg.Unwrap()));
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseBracket }
            );
        }

        if (parser.Peek() == TokenKind::CloseBracket)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBracket
            ));
        }

        if (args.empty())
        {
            return diagnostics.Add(CreateEmptyTemplateArgsError(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() }
            ));
        }

        return Expected{ std::move(args), diagnostics };
    }

    static auto ParseOptionalTemplateArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return Expected{ std::vector<SymbolName>{}, diagnostics };
        }

        const auto expArgs = ParseTemplateArgs(parser, scope);
        diagnostics.Add(expArgs);
        if (!expArgs)
        {
            return diagnostics;
        }

        return Expected{ std::move(expArgs.Unwrap()), diagnostics };
    }

    static auto ParseAttribute(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AttributeNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            );
        }

        parser.Eat();

        const auto expStructConstructionExpr = ParseStructConstructionExpr(
            parser,
            scope
        );
        diagnostics.Add(expStructConstructionExpr);
        if (!expStructConstructionExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            return CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            );
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const AttributeNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                expStructConstructionExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseAttributes(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const AttributeNode>>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const AttributeNode>> attributes{};
        while (parser.Peek() == TokenKind::OpenBracket)
        {
            const auto expAttribute = ParseAttribute(parser, scope);
            diagnostics.Add(expAttribute);
            if (expAttribute)
            {
                attributes.push_back(expAttribute.Unwrap());
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Inclusive,
                TokenKind::CloseBracket
            );
        }

        return Expected{ std::move(attributes), diagnostics };
    }

    static auto ParseParam(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const size_t index
    ) -> Expected<std::shared_ptr<const NormalParamVarNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            );
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const NormalParamVarNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                index
            ),
            diagnostics,
        };
    }

    static auto ParseParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalParamVarNode>>>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const NormalParamVarNode>> params{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseParen)
            )
        {
            if (!params.empty())
            {
                if (parser.Peek() == TokenKind::Comma)
                {
                    parser.Eat();
                }
                else
                {
                    diagnostics.Add(CreateMissingTokenError(
                        parser.GetLastSrcLocation(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expParam = ParseParam(
                parser,
                scope,
                params.size()
            );
            diagnostics.Add(expParam);
            if (expParam)
            {
                params.push_back(expParam.Unwrap());
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseParen }
            );
        }

        if (parser.Peek() == TokenKind::CloseParen)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseParen
            ));
        }

        return Expected{ std::move(params), diagnostics };
    }

    static auto ParseArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const IExprNode>>>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const IExprNode>> args{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseParen)
            )
        {
            if (!args.empty())
            {
                if (parser.Peek() == TokenKind::Comma)
                {
                    parser.Eat();
                }
                else
                {
                    diagnostics.Add(CreateMissingTokenError(
                        parser.GetLastSrcLocation(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expArg = ParseExpr(parser, scope);
            diagnostics.Add(expArg);
            if (expArg)
            {
                args.push_back(expArg.Unwrap());
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseParen }
            );
        }

        if (parser.Peek() == TokenKind::CloseParen)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseParen
            ));
        }

        return Expected{ std::move(args), diagnostics };
    }

    static auto ParseLiteralExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const LiteralExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto& literalToken = parser.Eat();

        const auto expLiteralKind = GetLiteralKind(literalToken);
        diagnostics.Add(expLiteralKind);
        if (!expLiteralKind)
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const LiteralExprNode>(
                literalToken->SrcLocation,
                scope,
                expLiteralKind.Unwrap(),
                literalToken->String
            ),
            diagnostics,
        };
    }

    static auto ParseSymbolLiteralExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SymbolLiteralExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expName = ParseSymbolName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const SymbolLiteralExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expName.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseStructConstructionExprArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<StructConstructionExprArg>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        parser.Eat();

        std::vector<StructConstructionExprArg> args{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (!args.empty())
            {
                if (parser.Peek() == TokenKind::Comma)
                {
                    parser.Eat();
                }
                else
                {
                    diagnostics.Add(CreateMissingTokenError(
                        parser.GetLastSrcLocation(),
                        TokenKind::Comma
                    ));
                }

                if (parser.Peek() == TokenKind::CloseBrace)
                {
                    break;
                }
            }

            const auto expName = ParseName(parser, scope);
            diagnostics.Add(expName);
            if (!expName)
            {
                return diagnostics;
            }

            std::optional<std::shared_ptr<const IExprNode>> optValue{};
            if (parser.Peek() == TokenKind::Colon)
            {
                parser.Eat();

                const auto expValue = ParseExpr(parser, scope);
                diagnostics.Add(expValue);
                if (!expValue)
                {
                    return diagnostics;
                }

                optValue = expValue.Unwrap();
            }
            
            args.emplace_back(
                expName.Unwrap(),
                optValue
            );
        }

        if (parser.Peek() == TokenKind::CloseBrace)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBrace
            ));
        }

        return Expected{ std::move(args), diagnostics };
    }

    static auto ParseStructConstructionExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructConstructionExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != SpecialIdent::New)
        {
            return diagnostics.Add(CreateUnexpectedTokenExpectedNewError(
                parser.Peek()
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseSymbolName(parser, scope);
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto expArgs = ParseStructConstructionExprArgs(
            parser,
            scope
        );
        diagnostics.Add(expArgs);
        if (!expArgs)
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const StructConstructionExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expTypeName.Unwrap(),
                std::move(expArgs.Unwrap())
            ),
            diagnostics,
        };
    }

    static auto ParseCastExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const CastExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::CastKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CastKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnostics.Add(expExpr);
        if (!expExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const CastExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                expTypeName.Unwrap(),
                expExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseAddressOfExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AddressOfExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::AddressOfKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::AddressOfKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnostics.Add(expExpr);
        if (!expExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const AddressOfExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                expExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseSizeOfExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SizeOfExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::SizeOfKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::SizeOfKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const SizeOfExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expTypeName.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseDerefAsExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const DerefAsExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::DerefAsKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::DerefAsKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnostics.Add(expExpr);
        if (!expExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const DerefAsExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                expTypeName.Unwrap(),
                expExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseExprExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExprExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnostics.Add(expExpr);
        if (!expExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExprExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                expExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseKeywordExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        switch (parser.Peek()->Kind)
        {
            case TokenKind::CastKeyword:
            {
                return ParseCastExpr(parser, scope);
            }

            case TokenKind::AddressOfKeyword:
            {
                return ParseAddressOfExpr(parser, scope);
            }

            case TokenKind::SizeOfKeyword:
            {
                return ParseSizeOfExpr(parser, scope);
            }

            case TokenKind::DerefAsKeyword:
            {
                return ParseDerefAsExpr(parser, scope);
            }

            default:
            {
                return DiagnosticBag{}.Add(CreateUnexpectedTokenError(
                    parser.Peek()
                ));
            }
        }
    }

    static auto ParsePrimaryExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        if (IsKeywordExprBegin(parser))
        {
            return ParseKeywordExpr(parser, scope);
        }

        if (IsExprExprBegin(parser))
        {
            return ParseExprExpr(parser, scope);
        }

        if (IsStructConstructionExprBegin(parser))
        {
            return ParseStructConstructionExpr(parser, scope);
        }

        if (IsLiteralExprBegin(parser))
        {
            return ParseLiteralExpr(parser, scope);
        }

        if (IsSymbolLiteralExprBegin(parser))
        {
            return ParseSymbolLiteralExpr(parser, scope);
        }

        return DiagnosticBag{}.Add(CreateUnexpectedTokenError(
            parser.Peek()
        ));
    }

    static auto ParseSecondaryExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expPrimaryExpr = ParsePrimaryExpr(parser, scope);
        diagnostics.Add(expPrimaryExpr);
        if (!expPrimaryExpr)
        {
            return diagnostics;
        }

        auto expr = expPrimaryExpr.Unwrap();

        while (
            (parser.Peek() == TokenKind::Dot) ||
            (parser.Peek() == TokenKind::OpenParen)
            )
        {
            if (parser.Peek() == TokenKind::Dot)
            {
                parser.Eat();

                const auto expName = ParseSymbolNameSection(parser, scope);
                diagnostics.Add(expName);
                if (!expName)
                {
                    return diagnostics;
                }

                expr = std::make_shared<const MemberAccessExprNode>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    expr,
                    expName.Unwrap()
                );
            }

            if (parser.Peek() == TokenKind::OpenParen)
            {
                const auto expArgs = ParseArgs(parser, scope);
                diagnostics.Add(expArgs);
                if (!expArgs)
                {
                    return diagnostics;
                }

                expr = std::make_shared<const FunctionCallExprNode>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    expr,
                    expArgs.Unwrap()
                );
            }
        }

        return Expected
        {
            expr,
            diagnostics,
        };
    }

    static auto ParseUnaryExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<Op> ops{};
        while (IsPrefixOp(parser.Peek()->Kind))
        {
            const auto& opToken = parser.Eat();
            ops.emplace_back(
                opToken->SrcLocation,
                opToken->Kind
            );
        }

        const auto expSecondaryExpr = ParseSecondaryExpr(parser, scope);
        diagnostics.Add(expSecondaryExpr);
        if (!expSecondaryExpr)
        {
            return diagnostics;
        }

        auto expr = expSecondaryExpr.Unwrap();
        while (!ops.empty())
        {
            const auto& op = ops.back();

            const SrcLocation srcLocation
            {
                op.SrcLocation,
                parser.GetLastSrcLocation(),
            };
            expr = CreateCollapsedPrefixExpr(
                srcLocation,
                expr,
                op
            );

            ops.pop_back();
        }

        return Expected
        {
            expr,
            diagnostics,
        };
    }

    static auto ParseExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnostics{};

        const auto expUnaryExpr = ParseUnaryExpr(parser, scope);
        diagnostics.Add(expUnaryExpr);
        if (!expUnaryExpr)
        {
            return diagnostics;
        }

        std::vector<std::shared_ptr<const IExprNode>> exprs{};
        std::vector<Op> ops{};

        exprs.push_back(expUnaryExpr.Unwrap());
        
        while (IsBinaryOp(parser.Peek()->Kind))
        {
            const auto& opToken = parser.Eat();
            ops.emplace_back(
                opToken->SrcLocation,
                opToken->Kind
            );

            const auto expUnaryExpr = ParseUnaryExpr(parser, scope);
            diagnostics.Add(expUnaryExpr);
            if (!expUnaryExpr)
            {
                return diagnostics;
            }

            exprs.push_back(expUnaryExpr.Unwrap());
        }

        if (ops.empty())
        {
            ACE_ASSERT(exprs.size() == 1);

            return Expected
            {
                exprs.front(),
                diagnostics,
            };
        }

        for (
            ssize_t precedenceLevel = MaxBinaryOpPrecedence;
            precedenceLevel >= 0;
            precedenceLevel--
            )
        {
            bool didCollapseAny = true;
            while (didCollapseAny)
            {
                didCollapseAny = false;

                for (size_t i = 0; i < ops.size(); i++)
                {
                    const auto op = ops.at(i);
                    const auto precedence =
                        GetBinaryOpPrecedence(op.TokenKind);

                    if (precedence == precedenceLevel)
                    {
                        didCollapseAny = true;

                        const auto lhsExpr = exprs.at(i);
                        const auto rhsExpr = exprs.at(i + 1);

                        ops.erase(begin(ops) + i);
                        exprs.erase(begin(exprs) + i + 1);
                        exprs.at(i) = CreateCollapsedBinaryExpr(
                            CreateSrcLocationRange(lhsExpr, rhsExpr),
                            lhsExpr,
                            rhsExpr,
                            op
                        );

                        break;
                    }
                }
            }
            
        }

        ACE_ASSERT(exprs.size() == 1);
        ACE_ASSERT(ops.empty());

        return Expected
        {
            exprs.front(),
            diagnostics,
        };
    }

    static auto ParseBlockStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const BlockStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const IStmtNode>> stmts{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBrace)
            )
        {
            const auto expStmt = ParseStmt(parser, selfScope);
            diagnostics.Add(expStmt);
            if (expStmt)
            {
                stmts.push_back(expStmt.Unwrap());
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Inclusive,
                TokenKind::Semicolon
            );
        }

        if (parser.Peek() == TokenKind::CloseBrace)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const BlockStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                selfScope,
                stmts
            ),
            diagnostics,
        };
    }

    static auto ParseExprStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExprStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expExpr = ParseExpr(parser, scope);
        diagnostics.Add(expExpr);
        if (!expExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExprStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                expExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseAssignmentStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const NormalAssignmentStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expLhsExpr = ParseExpr(parser, scope);
        diagnostics.Add(expLhsExpr);
        if (!expLhsExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Equals)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Equals
            ));
        }

        parser.Eat();

        const auto expRhsExpr = ParseExpr(parser, scope);
        diagnostics.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const NormalAssignmentStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expLhsExpr.Unwrap(),
                expRhsExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseCompoundAssignmentStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const CompoundAssignmentStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expLhsExpr = ParseExpr(parser, scope);
        diagnostics.Add(expLhsExpr);
        if (!expLhsExpr)
        {
            return diagnostics;
        }

        if (!IsCompoundAssignmentOp(parser.Peek()->Kind))
        {
            return diagnostics.Add(
                CreateUnexpectedTokenExpectedCompoundAssignmentOpError(parser.Peek())
            );
        }

        const auto opToken = parser.Eat();
        const Op op
        {
            opToken->SrcLocation,
            opToken->Kind,
        };

        const auto expRhsExpr = ParseExpr(parser, scope);
        diagnostics.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const CompoundAssignmentStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expLhsExpr.Unwrap(),
                expRhsExpr.Unwrap(),
                op
            ),
            diagnostics,
        };
    }

    static auto ParseVarStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const VarStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        std::optional<std::shared_ptr<const IExprNode>> optAssignment{};
        if (parser.Peek() == TokenKind::Equals)
        {
            parser.Eat();

            const auto expExpr = ParseExpr(parser, scope);
            diagnostics.Add(expExpr);
            if (!expExpr)
            {
                return diagnostics;
            }

            optAssignment = expExpr.Unwrap();
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const VarStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                optAssignment
            ),
            diagnostics,
        };
    }

    static auto ParseIfBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::IfKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::IfKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnostics.Add(expCondition);
        if (!expCondition)
        {
            return diagnostics;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        return Expected
        {
            std::pair
            {
                expCondition.Unwrap(),
                expBody.Unwrap()
            },
            diagnostics,
        };
    }

    static auto ParseElifBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::ElifKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ElifKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnostics.Add(expCondition);
        if (!expCondition)
        {
            return diagnostics;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        return Expected
        {
            std::pair
            {
                expCondition.Unwrap(),
                expBody.Unwrap()
            },
            diagnostics,
        };
    }

    static auto ParseElseBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const BlockStmtNode>>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::ElseKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ElseKeyword
            ));
        }

        parser.Eat();

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        return Expected{ expBody.Unwrap(), diagnostics };
    }

    static auto ParseIfStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IfStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        std::vector<std::shared_ptr<const IExprNode>> conditions{};
        std::vector<std::shared_ptr<const BlockStmtNode>> bodies{};

        const auto expIfBlock = ParseIfBlock(parser, scope);
        diagnostics.Add(expIfBlock);
        if (!expIfBlock)
        {
            return diagnostics;
        }

        conditions.push_back(expIfBlock.Unwrap().first);
        bodies.push_back(expIfBlock.Unwrap().second);

        while (parser.Peek() == TokenKind::ElifKeyword)
        {
            const auto expElifBlock = ParseElifBlock(parser, scope);
            diagnostics.Add(expElifBlock);
            if (!expElifBlock)
            {
                return diagnostics;
            }

            conditions.push_back(expElifBlock.Unwrap().first);
            bodies.push_back(expElifBlock.Unwrap().second);
        }

        if (parser.Peek() == TokenKind::ElseKeyword)
        {
            const auto expElseBlock = ParseElseBlock(parser, scope);
            diagnostics.Add(expElseBlock);
            if (!expElseBlock)
            {
                return diagnostics;
            }

            bodies.push_back(expElseBlock.Unwrap());
        }

        return Expected
        {
            std::make_shared<const IfStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                conditions,
                bodies
            ),
            diagnostics,
        };
    }

    static auto ParseWhileStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const WhileStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::WhileKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::WhileKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnostics.Add(expCondition);
        if (!expCondition)
        {
            return diagnostics;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const WhileStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expCondition.Unwrap(),
                expBody.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseReturnStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ReturnStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::ReturnKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ReturnKeyword
            ));
        }

        parser.Eat();

        std::optional<std::shared_ptr<const IExprNode>> optExpr{};
        if (parser.Peek() != TokenKind::Semicolon)
        {
            const auto expExpr = ParseExpr(parser, scope);
            diagnostics.Add(expExpr);
            if (!expExpr)
            {
                return diagnostics;
            }

            optExpr = expExpr.Unwrap();
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ReturnStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optExpr
            ),
            diagnostics,
        };
    }

    static auto ParseExitStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExitStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::ExitKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ExitKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExitStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope
            ),
            diagnostics,
        };
    }

    static auto ParseAssertStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AssertStmtNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::AssertKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::AssertKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnostics.Add(expCondition);
        if (!expCondition)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const AssertStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expCondition.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseKeywordStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IStmtNode>>
    {
        switch (parser.Peek()->Kind)
        {
            case TokenKind::IfKeyword:
            {
                return ParseIfStmt(parser, scope);
            }

            case TokenKind::WhileKeyword:
            {
                return ParseWhileStmt(parser, scope);
            }

            case TokenKind::ReturnKeyword:
            {
                return ParseReturnStmt(parser, scope);
            }

            case TokenKind::ExitKeyword:
            {
                return ParseExitStmt(parser, scope);
            }

            case TokenKind::AssertKeyword:
            {
                return ParseAssertStmt(parser, scope);
            }

            default:
            {
                DiagnosticBag diagnostics{};
                return diagnostics.Add(CreateUnexpectedTokenError(
                    parser.Peek()
                ));
            }
        }
    }

    static auto ParseStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IStmtNode>>
    {
        if (IsVarBegin(parser))
        {
            return ParseVarStmt(parser, scope);
        }
        else if (IsBlockStmtBegin(parser))
        {
            return ParseBlockStmt(parser, scope);
        }
        else if (IsKeywordStmtBegin(parser))
        {
            return ParseKeywordStmt(parser, scope);
        }

        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expExpr = ParseExpr(parser, scope);
        diagnostics.Add(expExpr);
        if (!expExpr)
        {
            return diagnostics;
        }

        const bool isSemicolon = parser.Peek() == TokenKind::Semicolon;
        const bool isAssignment = parser.Peek() == TokenKind::Equals;
        const bool isCompoundAssignment = IsCompoundAssignmentOp(
            parser.Peek()->Kind
        );

        if (
            isSemicolon ||
            (!isAssignment && !isCompoundAssignment)
            )
        {
            if (isSemicolon)
            {
                parser.Eat();
            }
            else
            {
                diagnostics.Add(CreateMissingTokenError(
                    parser.GetLastSrcLocation(),
                    TokenKind::Semicolon
                ));
            }

            return Expected
            {
                std::make_shared<const ExprStmtNode>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    expExpr.Unwrap()
                ),
                diagnostics,
            };
        }

        const auto opToken = parser.Eat();
        const Op op
        {
            opToken->SrcLocation,
            opToken->Kind,
        };
    
        const auto expRhsExpr = ParseExpr(parser, scope);
        diagnostics.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnostics;
        }

        if (parser.Peek() == TokenKind::Semicolon)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::Semicolon
            ));
        }

        if (isAssignment)
        {
            return Expected
            {
                std::make_shared<const NormalAssignmentStmtNode>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    scope,
                    expExpr.Unwrap(),
                    expRhsExpr.Unwrap()
                ),
                diagnostics,
            };
        }

        if (isCompoundAssignment)
        {
            return Expected
            {
                std::make_shared<const CompoundAssignmentStmtNode>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    scope,
                    expExpr.Unwrap(),
                    expRhsExpr.Unwrap(),
                    op
                ),
                diagnostics,
            };
        }

        ACE_UNREACHABLE();
    }

    static auto ParseFunctionOrOpNameToken(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<FunctionOrOpNameToken>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() == TokenKind::Ident)
        {
            return Expected
            {
                FunctionOrOpNameToken
                {
                    FunctionOrOpNameKind::Function,
                    parser.Eat(),
                },
                diagnostics,
            };
        }

        if (parser.Peek() != TokenKind::OpKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Ident
            ));
        }

        parser.Eat();

        if (
            (parser.Peek() != TokenKind::Ident) &&
            !IsUserOp(parser.Peek()->Kind)
            )
        {
            return diagnostics.Add(
                CreateUnexpectedTokenExpectedOverloadableOpError(parser.Peek())
            );
        }

        return Expected
        {
            FunctionOrOpNameToken
            {
                FunctionOrOpNameKind::Op,
                parser.Eat(),
            },
            diagnostics,
        };
    }

    static auto ParseModifiersUntil(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        std::vector<Modifier> allowedModifiers,
        const std::function<bool()>& predicate
    ) -> Diagnosed<std::map<Modifier, std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnostics{};

        ACE_ASSERT(parser.Peek() == TokenKind::MinusGreaterThan);
        const auto& minusGreaterThanToken = parser.Eat();

        std::map<Modifier, std::shared_ptr<const Token>> modifierToTokenMap{};
        while (!parser.IsEnd() && !predicate())
        {
            const auto modifierToken = parser.Eat();

            const auto expModifier = GetModifier(modifierToken);
            diagnostics.Add(expModifier);
            if (!expModifier)
            {
                continue;
            }

            const auto allowedModifierIt = std::find(
                begin(allowedModifiers),
                end  (allowedModifiers),
                expModifier.Unwrap()
            );
            if (allowedModifierIt == end(allowedModifiers))
            {
                diagnostics.Add(CreateForbiddenModifierError(
                    modifierToken
                ));
                continue;
            }

            allowedModifiers.erase(
                begin(allowedModifiers),
                allowedModifierIt
            );

            modifierToTokenMap[expModifier.Unwrap()] = modifierToken;
        }

        if (parser.Peek().get() == minusGreaterThanToken.get())
        {
            diagnostics.Add(CreateEmptyModifiersError(
                minusGreaterThanToken
            ));
        }

        return Diagnosed
        {
            std::move(modifierToTokenMap),
            diagnostics,
        };
    }

    static auto ParseImplFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName
    ) -> Expected<std::shared_ptr<const FunctionNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expNameToken = ParseFunctionOrOpNameToken(
            parser,
            scope
        );
        diagnostics.Add(expNameToken);
        if (!expNameToken)
        {
            return diagnostics;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnostics.Add(expParams);
        if (!expParams)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        bool isExtern = false;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Self, Modifier::Extern },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }

            if (modifierToTokenMap.contains(Modifier::Self))
            {
                optSelfToken = modifierToTokenMap.at(Modifier::Self);
            }

            if (modifierToTokenMap.contains(Modifier::Extern))
            {
                isExtern = true;

                if (modifierToTokenMap.contains(Modifier::Self))
                {
                    diagnostics.Add(CreateExternInstanceFunctionError(
                        modifierToTokenMap.at(Modifier::Extern)
                    ));
                }
            }
        }

        const auto expName = CreateFunctionOrOpName(
            expNameToken.Unwrap(),
            expParams.Unwrap().size(),
            accessModifier,
            optSelfToken.has_value()
        );
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        std::optional<std::shared_ptr<const BlockStmtNode>> optBody{};
        if (isExtern)
        {
            if (parser.Peek() != TokenKind::Semicolon)
            {
                return diagnostics.Add(CreateUnexpectedTokenError(
                    parser.Peek(),
                    TokenKind::Semicolon
                ));
            }

            parser.Eat();
        }
        else
        {
            const auto expBody = ParseBlockStmt(parser, selfScope);
            diagnostics.Add(expBody);
            if (!expBody)
            {
                return diagnostics;
            }

            optBody = expBody.Unwrap();
        }

        const auto optSelfParam = CreateSelfParam(
            optSelfToken,
            selfScope,
            selfTypeName
        );

        return Expected
        {
            std::make_shared<const FunctionNode>(
                SrcLocation{ beginSrcLocation, parser.GetSrcLocation() },
                selfScope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                optSelfParam,
                expParams.Unwrap(),
                optBody
            ),
            diagnostics,
        };
    }

    static auto ParseImplFunctionTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        const auto expTemplateParams = ParseTemplateParams(
            parser,
            selfScope
        );
        diagnostics.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnostics;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnostics.Add(expParams);
        if (!expParams)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Self },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }

            if (modifierToTokenMap.contains(Modifier::Self))
            {
                optSelfToken = modifierToTokenMap.at(Modifier::Self);
            }
        }

        const auto expBody = ParseBlockStmt(parser, selfScope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        const auto selfParam = CreateSelfParam(
            optSelfToken,
            selfScope,
            selfTypeName
        );

        const auto function = std::make_shared<const FunctionNode>(
            SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
            selfScope,
            expName.Unwrap(),
            expTypeName.Unwrap(),
            expAttributes.Unwrap(),
            accessModifier,
            selfParam,
            expParams.Unwrap(),
            expBody.Unwrap()
        );

        return Expected
        {
            std::make_shared<const FunctionTemplateNode>(
                function->GetSrcLocation(),
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                expTemplateParams.Unwrap(),
                function
            ),
            diagnostics,
        };
    }

    static auto ParseImpl(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const NormalImplNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        if (parser.Peek() != TokenKind::ImplKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ImplKeyword
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseSymbolName(parser, scope);
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        // TODO: Remove this block after impl template specialization
        {
            const auto& sections = expTypeName.Unwrap().Sections;

            const auto sectionWithTemplateArgsIt = std::find_if_not(
                begin(sections),
                end  (sections),
                [](const SymbolNameSection& section)
                {
                    return section.TemplateArgs.empty();
                }
            );
            if (sectionWithTemplateArgsIt != end(sections))
            {
                const auto firstSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.front().Name.SrcLocation;
                const auto lastSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.back().Name.SrcLocation;

                const SrcLocation srcLocation
                {
                    firstSectionSoruceLocation.Buffer,
                    firstSectionSoruceLocation.CharacterBeginIterator,
                     lastSectionSoruceLocation.CharacterEndIterator,
                };

                return diagnostics.Add(CreateTemplateSpecializationError(
                    srcLocation
                ));
            }
        }

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (IsFunctionBegin(parser))
            {
                const auto expFunction = ParseImplFunction(
                    parser,
                    selfScope,
                    expTypeName.Unwrap()
                );
                diagnostics.Add(expFunction);
                if (expFunction)
                {
                    functions.push_back(expFunction.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionTemplateBegin(parser))
            {
                const auto expFunctionTemplate = ParseImplFunctionTemplate(
                    parser,
                    selfScope,
                    expTypeName.Unwrap()
                );
                diagnostics.Add(expFunctionTemplate);
                if (expFunctionTemplate)
                {
                    functionTemplates.push_back(expFunctionTemplate.Unwrap());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(
                    parser.Peek()
                ));
            }

            parser.DiscardUntil(
                DiscardKind::Inclusive,
                { TokenKind::CloseBrace, TokenKind::Semicolon }
            );
        }

        if (parser.Peek() == TokenKind::CloseBrace)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const NormalImplNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                selfScope,
                expTypeName.Unwrap(),
                functions,
                functionTemplates
            ),
            diagnostics,
        };
    }

    static auto ParseTemplatedImplFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName,
        const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& implTemplateParams
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expNameToken = ParseFunctionOrOpNameToken(
            parser,
            scope
        );
        diagnostics.Add(expNameToken);
        if (!expNameToken)
        {
            return diagnostics;
        }

        const auto expTemplateParams = ParseOptionalTemplateParams(
            parser,
            selfScope
        );
        diagnostics.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnostics;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnostics.Add(expParams);
        if (!expParams)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Self },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }

            if (modifierToTokenMap.contains(Modifier::Self))
            {
                optSelfToken = modifierToTokenMap.at(Modifier::Self);
            }
        }

        const auto expName = CreateFunctionOrOpName(
            expNameToken.Unwrap(),
            expParams.Unwrap().size(),
            accessModifier,
            optSelfToken.has_value()
        );
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        const auto optSelfParam = CreateSelfParam(
            optSelfToken,
            selfScope,
            selfTypeName
        );

        const auto function = std::make_shared<const FunctionNode>(
            SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
            selfScope,
            expName.Unwrap(),
            expTypeName.Unwrap(),
            expAttributes.Unwrap(),
            accessModifier,
            optSelfParam,
            expParams.Unwrap(),
            expBody.Unwrap()
        );

        std::vector<std::shared_ptr<const ImplTemplateParamNode>> clonedImplTemplateParams{};
        std::transform(
            begin(implTemplateParams),
            end  (implTemplateParams),
            back_inserter(clonedImplTemplateParams),
            [&](const std::shared_ptr<const ImplTemplateParamNode>& implTemplateParam)
            {
                return implTemplateParam->CloneInScope(selfScope);
            }
        );

        return Expected
        {
            std::make_shared<const FunctionTemplateNode>(
                function->GetSrcLocation(),
                clonedImplTemplateParams,
                expTemplateParams.Unwrap(),
                function
            ),
            diagnostics,
        };
    }

    static auto ParseTemplatedImpl(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TemplatedImplNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        if (parser.Peek() != TokenKind::ImplKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ImplKeyword
            ));
        }

        parser.Eat();

        const auto expTemplateParams = ParseImplTemplateParams(
            parser,
            selfScope
        );
        diagnostics.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnostics;
        }

        const auto expTypeName = ParseSymbolName(parser, scope);
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        // TODO: Remove this block after impl template specialization
        {
            const auto expIsNotSpecialized = [&]() -> Expected<void>
            {
                const auto& templateParams = expTemplateParams.Unwrap();
                const auto& typeNameSections = expTypeName.Unwrap().Sections;

                const bool foundTemplatedSection = std::find_if(
                    begin(typeNameSections),
                    end  (typeNameSections) - 1,
                    [](const SymbolNameSection& section)
                    {
                        return section.TemplateArgs.empty();
                    }
                ) == end(typeNameSections);

                ACE_TRY_ASSERT(!foundTemplatedSection);

                const auto& templateArgs = typeNameSections.back().TemplateArgs;
                ACE_TRY_ASSERT(templateParams.size() == templateArgs.size());

                std::unordered_set<std::string> templateParamSet{};
                ACE_TRY_VOID(TransformExpectedVector(templateParams,
                [&](const std::shared_ptr<const ImplTemplateParamNode>& templateParam) -> Expected<void>
                {
                    const std::string& templateParamName =
                        templateParam->GetName().String;

                    ACE_TRY_ASSERT(!templateParamSet.contains(
                        templateParamName
                    ));
                    templateParamSet.insert(templateParamName);

                    return Void{};
                }));
                
                ACE_TRY_VOID(TransformExpectedVector(templateArgs,
                [&](const SymbolName& arg) -> Expected<void>
                {
                    ACE_TRY_ASSERT(arg.Sections.size() == 1);
                    ACE_TRY_ASSERT(arg.Sections.back().TemplateArgs.empty());

                    const std::string& templateArgName =
                        arg.Sections.front().Name.String;
                    ACE_TRY_ASSERT(templateParamSet.contains(templateArgName));
                    templateParamSet.erase(templateArgName);

                    return Void{};
                }));

                return Void{};
            }();
            if (!expIsNotSpecialized)
            {
                const auto firstSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.front().Name.SrcLocation;
                const auto lastSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.back().Name.SrcLocation;

                const SrcLocation srcLocation
                {
                    firstSectionSoruceLocation.Buffer,
                    firstSectionSoruceLocation.CharacterBeginIterator,
                     lastSectionSoruceLocation.CharacterEndIterator,
                };

                return diagnostics.Add(CreateTemplateSpecializationError(
                    srcLocation
                ));
            }
        }

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (IsFunctionBegin(parser) || IsFunctionTemplateBegin(parser))
            {
                const auto expFunctionTemplate = ParseTemplatedImplFunction(
                    parser,
                    selfScope,
                    expTypeName.Unwrap(),
                    expTemplateParams.Unwrap()
                );
                diagnostics.Add(expFunctionTemplate);
                if (expFunctionTemplate)
                {
                    functionTemplates.push_back(expFunctionTemplate.Unwrap());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(
                    parser.Peek()
                ));
            }
            
            parser.DiscardUntil(
                DiscardKind::Inclusive,
                { TokenKind::Semicolon, TokenKind::CloseBrace }
            );
        }

        if (parser.Peek() == TokenKind::CloseBrace)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBrace
            ));
        }

        auto typeTemplateName = expTypeName.Unwrap();
        auto& typeTemplateNameLastSection = typeTemplateName.Sections.back();
        typeTemplateNameLastSection.TemplateArgs.clear();
        typeTemplateNameLastSection.Name.String = SpecialIdent::CreateTemplate(
            typeTemplateNameLastSection.Name.String
        );

        return Expected
        {
            std::make_shared<const TemplatedImplNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                selfScope,
                typeTemplateName,
                std::vector<std::shared_ptr<const FunctionNode>>{},
                functionTemplates
            ),
            diagnostics,
        };
    }

    static auto ParseFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const FunctionNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnostics.Add(expParams);
        if (!expParams)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            selfScope,
            RefParsingKind::Disallow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        bool isExtern = false;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Extern },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }

            if (modifierToTokenMap.contains(Modifier::Extern))
            {
                isExtern = true;
            }
        }

        std::optional<std::shared_ptr<const BlockStmtNode>> optBody{};
        if (isExtern)
        {
            if (parser.Peek() != TokenKind::Semicolon)
            {
                return diagnostics.Add(CreateUnexpectedTokenError(
                    parser.Peek(),
                    TokenKind::Semicolon
                ));
            }

            parser.Eat();
        }
        else
        {
            const auto expBody = ParseBlockStmt(parser, selfScope);
            diagnostics.Add(expBody);
            if (!expBody)
            {
                return diagnostics;
            }

            optBody = expBody.Unwrap();
        }

        return Expected
        {
            std::make_shared<const FunctionNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                selfScope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                std::nullopt,
                expParams.Unwrap(),
                optBody
            ),
            diagnostics,
        };
    }

    static auto ParseFunctionTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        const auto expTemplateParams = ParseTemplateParams(
            parser,
            selfScope
        );
        diagnostics.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnostics;
        }

        const auto expParams = ParseParams(
            parser,
            selfScope
        );
        diagnostics.Add(expParams);
        if (!expParams)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto expBody = ParseBlockStmt(parser, selfScope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        const auto function = std::make_shared<const FunctionNode>(
            SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
            selfScope,
            expName.Unwrap(),
            expTypeName.Unwrap(),
            expAttributes.Unwrap(),
            accessModifier,
            std::nullopt,
            expParams.Unwrap(),
            expBody.Unwrap()
        );

        return Expected
        {
            std::make_shared<const FunctionTemplateNode>(
                function->GetSrcLocation(),
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                expTemplateParams.Unwrap(),
                function
            ),
            diagnostics,
        };
    }

    static auto ParseVar(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StaticVarNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::Semicolon; }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const StaticVarNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier
            ),
            diagnostics,
        };
    }

    static auto ParseMemberVar(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const size_t index
    ) -> Expected<std::shared_ptr<const InstanceVarNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        );
        diagnostics.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::Comma; }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        return Expected
        {
            std::make_shared<const InstanceVarNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                index
            ),
            diagnostics,
        };
    }

    static auto ParseStructBody(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const InstanceVarNode>>>
    {
        DiagnosticBag diagnostics{};

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const InstanceVarNode>> vars{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (!vars.empty())
            {
                if (parser.Peek() == TokenKind::Comma)
                {
                    parser.Eat();
                }
                else
                {
                    diagnostics.Add(CreateMissingTokenError(
                        parser.GetLastSrcLocation(),
                        TokenKind::Comma
                    ));
                }

                if (parser.Peek() == TokenKind::CloseBrace)
                {
                    break;
                }
            }
            
            const auto expVar = ParseMemberVar(
                parser,
                scope,
                vars.size()
            );
            diagnostics.Add(expVar);
            if (expVar)
            {
                vars.push_back(expVar.Unwrap());
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseBrace }
            );
        }

        if (parser.Peek() == TokenKind::CloseBrace)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBrace
            ));
        }

        return Expected{ std::move(vars), diagnostics };
    }

    static auto ParseStruct(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructTypeNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::StructKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::StructKeyword
            ));
        }

        parser.Eat();

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::OpenBrace; }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }
        
        const auto expBody = ParseStructBody(parser, selfScope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const StructTypeNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                selfScope,
                expName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                expBody.Unwrap()
            ),
            diagnostics,
        };
    }

    static auto ParseStructTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TypeTemplateNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnostics.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnostics;
        }

        const auto expName = ParseName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        const auto expTemplateParams = ParseTemplateParams(
            parser,
            selfScope
        );
        diagnostics.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::StructKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::StructKeyword
            ));
        }

        parser.Eat();

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::OpenBrace; }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto expBody = ParseStructBody(parser, selfScope);
        diagnostics.Add(expBody);
        if (!expBody)
        {
            return diagnostics;
        }

        const auto type = std::make_shared<const StructTypeNode>(
            SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
            selfScope,
            expName.Unwrap(),
            expAttributes.Unwrap(),
            accessModifier,
            expBody.Unwrap()
        );

        return Expected
        {
            std::make_shared<const TypeTemplateNode>(
                type->GetSrcLocation(),
                expTemplateParams.Unwrap(),
                type
            ),
            diagnostics,
        };
    }

    static auto ParseTypeTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TypeTemplateNode>>
    {
        DiagnosticBag diagnostics{};

        if (IsStructTemplateBegin(parser))
        {
            const auto expStructTemplate = ParseStructTemplate(
                parser,
                scope
            );
            diagnostics.Add(expStructTemplate);
            if (!expStructTemplate)
            {
                return diagnostics;
            }

            return Expected
            {
                expStructTemplate.Unwrap(),
                diagnostics,
            };
        }

        return diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
    }

    static auto ParseType(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ITypeNode>>
    {
        DiagnosticBag diagnostics{};

        if (IsStructBegin(parser))
        {
            const auto expStruct = ParseStruct(parser, scope);
            diagnostics.Add(expStruct);
            if (!expStruct)
            {
                return expStruct;
            }

            return Expected{ expStruct.Unwrap(), diagnostics };
        }

        return diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
    }

    static auto ParseModule(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnostics{};

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto expName = ParseNestedName(parser, scope);
        diagnostics.Add(expName);
        if (!expName)
        {
            return diagnostics;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::ModuleKeyword)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ModuleKeyword
            ));
        }

        parser.Eat();

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::OpenBrace; }
            );
            diagnostics.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        std::vector<std::shared_ptr<Scope>> scopes{};
        scopes.push_back(scope);
        std::transform(
            begin(expName.Unwrap()),
            end  (expName.Unwrap()),
            back_inserter(scopes),
            [&](const Ident& name)
            {
                return scopes.back()->GetOrCreateChild(name.String);
            }
        );

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const ModuleNode>> modules{};
        std::vector<std::shared_ptr<const ITypeNode>> types{};
        std::vector<std::shared_ptr<const TypeTemplateNode>> typeTemplates{};
        std::vector<std::shared_ptr<const NormalImplNode>> normalImpls{};
        std::vector<std::shared_ptr<const TemplatedImplNode>> templatedImpls{};
        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        std::vector<std::shared_ptr<const StaticVarNode>> vars{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::CloseBrace)
            )
        {
            const auto selfScope = scopes.back();

            if (IsModuleBegin(parser))
            {
                const auto expModule = ParseModule(parser, selfScope);
                diagnostics.Add(expModule);
                if (expModule)
                {
                    modules.push_back(expModule.Unwrap());
                    continue;
                }
            }
            else if (IsTypeBegin(parser))
            {
                const auto expType = ParseType(parser, selfScope);
                diagnostics.Add(expType);
                if (expType)
                {
                    types.push_back(expType.Unwrap());
                    continue;
                }
            }
            else if (IsTypeTemplateBegin(parser))
            {
                const auto expTypeTemplate = ParseTypeTemplate(
                    parser,
                    selfScope
                );
                diagnostics.Add(expTypeTemplate);
                if (expTypeTemplate)
                {
                    typeTemplates.push_back(expTypeTemplate.Unwrap());
                    continue;
                }
            }
            else if (IsImplBegin(parser))
            {
                const auto expImpl = ParseImpl(
                    parser,
                    selfScope
                );
                diagnostics.Add(expImpl);
                if (expImpl)
                {
                    normalImpls.push_back(expImpl.Unwrap());
                    continue;
                }
            }
            else if (IsTemplatedImplBegin(parser))
            {
                const auto expTemplatedImpl = ParseTemplatedImpl(
                    parser,
                    selfScope
                );
                diagnostics.Add(expTemplatedImpl);
                if (expTemplatedImpl)
                {
                    templatedImpls.push_back(expTemplatedImpl.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionBegin(parser))
            {
                const auto expFunction = ParseFunction(parser, selfScope);
                diagnostics.Add(expFunction);
                if (expFunction)
                {
                    functions.push_back(expFunction.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionTemplateBegin(parser))
            {
                const auto expFunctionTemplate = ParseFunctionTemplate(
                    parser,
                    selfScope
                );
                diagnostics.Add(expFunctionTemplate);
                if (expFunctionTemplate)
                {
                    functionTemplates.push_back(expFunctionTemplate.Unwrap());
                    continue;
                }
            }
            else if (IsVarBegin(parser))
            {
                const auto expVar = ParseVar(parser, selfScope);
                diagnostics.Add(expVar);
                if (expVar)
                {
                    vars.push_back(expVar.Unwrap());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(
                    parser.Peek()
                ));
            }
            
            parser.DiscardUntil(
                DiscardKind::Inclusive,
                { TokenKind::Semicolon, TokenKind::CloseBrace }
            );
        }

        if (parser.Peek() == TokenKind::CloseBrace)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateMissingTokenError(
                parser.GetLastSrcLocation(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const ModuleNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scopes.front(),
                scopes.back(),
                expName.Unwrap(),
                accessModifier,
                modules,
                types,
                typeTemplates,
                normalImpls,
                templatedImpls,
                functions,
                functionTemplates,
                vars
            ),
            diagnostics,
        };
    }

    static auto ParseAST(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnostics{};

        const auto expModule = ParseModule(parser, scope);
        diagnostics.Add(expModule);
        if (!expModule)
        {
            return diagnostics;
        }

        ACE_ASSERT(parser.IsEnd());

        return
        {
            expModule.Unwrap(),
            diagnostics,
        };
    }

    auto ParseAST(
        const FileBuffer* const fileBuffer,
        const std::vector<std::shared_ptr<const Token>>& tokens
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        Parser parser{ fileBuffer, tokens };
        return ParseAST(
            parser,
            fileBuffer->GetCompilation()->GlobalScope.Unwrap()
        );
    }
}
