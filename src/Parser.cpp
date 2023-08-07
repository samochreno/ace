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

        const auto optName = diagnostics.Collect(GetOpFunctionName(
            nameToken.Value,
            paramCount
        ));
        if (!optName.has_value())
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
            optName.value(),
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
                fileBuffer->GetCompilation()->GetPackage().Name
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

            if (*(m_Iterator - 1) == TokenKind::OpenBrace)
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
            std::optional{ i } :
            std::nullopt;
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

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        nestedName.push_back(std::move(optName.value()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto optName = diagnostics.Collect(ParseName(parser, scope));
            if (!optName.has_value())
            {
                return diagnostics;
            }

            nestedName.push_back(std::move(optName.value()));
        }

        return Expected{ std::move(nestedName), diagnostics };
    }

    static auto ParseSymbolNameSection(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolNameSection>
    {
        DiagnosticBag diagnostics{};

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        const auto optTemplateArgs = diagnostics.Collect(ParseOptionalTemplateArgs(
            parser,
            scope
        ));
        if (!optTemplateArgs.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            SymbolNameSection
            {
                optName.value(),
                optTemplateArgs.value(),
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

        const auto optSection = diagnostics.Collect(ParseSymbolNameSection(
            parser,
            scope
        ));
        if (!optSection.has_value())
        {
            return diagnostics;
        }

        sections.push_back(std::move(optSection.value()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto optSection = diagnostics.Collect(ParseSymbolNameSection(
                parser,
                scope
            ));
            if (!optSection.has_value())
            {
                return diagnostics;
            }

            sections.push_back(std::move(optSection.value()));
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

        const auto optSymbolName = diagnostics.Collect(ParseSymbolName(
            parser,
            scope
        ));
        if (!optSymbolName.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            TypeName
            {
                optSymbolName.value(),
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

            const auto optName = diagnostics.Collect(ParseName(parser, scope));
            if (optName.has_value())
            {
                names.push_back(std::move(optName.value()));
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

        const auto optNames = diagnostics.Collect(ParseTemplateParamNames(
            parser,
            scope
        ));
        if (!optNames.has_value())
        {
            return diagnostics;
        }
        
        std::vector<std::shared_ptr<const ImplTemplateParamNode>> params{};
        std::transform(
            begin(optNames.value()),
            end  (optNames.value()),
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

        const auto optNames = diagnostics.Collect(ParseTemplateParamNames(
            parser,
            scope
        ));
        if (!optNames.has_value())
        {
            return diagnostics;
        }
        
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> params{};
        std::transform(
            begin(optNames.value()),
            end  (optNames.value()),
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

            const auto optArg = diagnostics.Collect(ParseSymbolName(
                parser,
                scope
            ));
            if (optArg.has_value())
            {
                args.push_back(std::move(optArg.value()));
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

        const auto optArgs = diagnostics.Collect(ParseTemplateArgs(
            parser,
            scope
        ));
        if (!optArgs.has_value())
        {
            return diagnostics;
        }

        return Expected{ std::move(optArgs.value()), diagnostics };
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
            return diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        parser.Eat();

        const auto optStructConstructionExpr = diagnostics.Collect(ParseStructConstructionExpr(
            parser,
            scope
        ));
        if (!optStructConstructionExpr.has_value())
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
            std::make_shared<const AttributeNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optStructConstructionExpr.value()
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
            const auto optAttribute = diagnostics.Collect(ParseAttribute(
                parser,
                scope
            ));
            if (optAttribute.has_value())
            {
                attributes.push_back(optAttribute.value());
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const NormalParamVarNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optName.value(),
                optTypeName.value(),
                optAttributes.value(),
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

            const auto optParam = diagnostics.Collect(ParseParam(
                parser,
                scope,
                params.size()
            ));
            if (optParam.has_value())
            {
                params.push_back(optParam.value());
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

            const auto optArg = diagnostics.Collect(ParseExpr(parser, scope));
            if (optArg.has_value())
            {
                args.push_back(optArg.value());
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

        const auto optLiteralKind = diagnostics.Collect(GetLiteralKind(literalToken));
        if (!optLiteralKind.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const LiteralExprNode>(
                literalToken->SrcLocation,
                scope,
                optLiteralKind.value(),
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

        const auto optName = diagnostics.Collect(ParseSymbolName(
            parser,
            scope
        ));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const SymbolLiteralExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optName.value()
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

            const auto optName = diagnostics.Collect(ParseName(parser, scope));
            if (!optName.has_value())
            {
                return diagnostics;
            }

            std::optional<std::shared_ptr<const IExprNode>> optValue{};
            if (parser.Peek() == TokenKind::Colon)
            {
                parser.Eat();

                optValue = diagnostics.Collect(ParseExpr(
                    parser,
                    scope
                ));
                if (!optValue.has_value())
                {
                    return diagnostics;
                }
            }
            
            args.emplace_back(
                optName.value(),
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

        const auto optTypeName = diagnostics.Collect(ParseSymbolName(
            parser,
            scope
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto optArgs = diagnostics.Collect(ParseStructConstructionExprArgs(
            parser,
            scope
        ));
        if (!optArgs.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const StructConstructionExprNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optTypeName.value(),
                std::move(optArgs.value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        ));
        if (!optTypeName.has_value())
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

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
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
                optTypeName.value(),
                optExpr.value()
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

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
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
                optExpr.value()
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        ));
        if (!optTypeName.has_value())
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
                optTypeName.value()
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        ));
        if (!optTypeName.has_value())
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

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
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
                optTypeName.value(),
                optExpr.value()
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

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
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
                optExpr.value()
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

        const auto optPrimaryExpr = diagnostics.Collect(ParsePrimaryExpr(parser, scope));
        if (!optPrimaryExpr.has_value())
        {
            return diagnostics;
        }

        auto expr = optPrimaryExpr.value();

        while (
            (parser.Peek() == TokenKind::Dot) ||
            (parser.Peek() == TokenKind::OpenParen)
            )
        {
            if (parser.Peek() == TokenKind::Dot)
            {
                parser.Eat();

                const auto optName = diagnostics.Collect(ParseSymbolNameSection(
                    parser,
                    scope
                ));
                if (!optName.has_value())
                {
                    return diagnostics;
                }

                expr = std::make_shared<const MemberAccessExprNode>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    expr,
                    optName.value()
                );
            }

            if (parser.Peek() == TokenKind::OpenParen)
            {
                const auto optArgs = diagnostics.Collect(ParseArgs(
                    parser,
                    scope
                ));
                if (!optArgs.has_value())
                {
                    return diagnostics;
                }

                expr = std::make_shared<const FunctionCallExprNode>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    expr,
                    optArgs.value()
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

        const auto optSecondaryExpr = diagnostics.Collect(ParseSecondaryExpr(
            parser,
            scope
        ));
        if (!optSecondaryExpr.has_value())
        {
            return diagnostics;
        }

        auto expr = optSecondaryExpr.value();
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

        const auto optUnaryExpr = diagnostics.Collect(ParseUnaryExpr(
            parser,
            scope
        ));
        if (!optUnaryExpr.has_value())
        {
            return diagnostics;
        }

        std::vector<std::shared_ptr<const IExprNode>> exprs{};
        std::vector<Op> ops{};

        exprs.push_back(optUnaryExpr.value());
        
        while (IsBinaryOp(parser.Peek()->Kind))
        {
            const auto& opToken = parser.Eat();
            ops.emplace_back(
                opToken->SrcLocation,
                opToken->Kind
            );

            const auto optUnaryExpr = diagnostics.Collect(ParseUnaryExpr(
                parser,
                scope
            ));
            if (!optUnaryExpr.has_value())
            {
                return diagnostics;
            }

            exprs.push_back(optUnaryExpr.value());
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
            const auto optStmt = diagnostics.Collect(ParseStmt(
                parser,
                selfScope
            ));
            if (optStmt.has_value())
            {
                stmts.push_back(optStmt.value());
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Semicolon, TokenKind::CloseBrace }
            );

            if (parser.Peek() == TokenKind::Semicolon)
            {
                parser.Eat();
            }
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

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
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
                optExpr.value()
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

        const auto optLHSExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optLHSExpr.has_value())
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

        const auto optRHSExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optRHSExpr.has_value())
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
                optLHSExpr.value(),
                optRHSExpr.value()
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

        const auto optLhsExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optLhsExpr.has_value())
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

        const auto optRhsExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optRhsExpr.has_value())
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
                optLhsExpr.value(),
                optRhsExpr.value(),
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

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        std::optional<std::shared_ptr<const IExprNode>> optAssignedExpr{};
        if (parser.Peek() == TokenKind::Equals)
        {
            parser.Eat();

            optAssignedExpr = diagnostics.Collect(ParseExpr(parser, scope));
            if (!optAssignedExpr.has_value())
            {
                return diagnostics;
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
            std::make_shared<const VarStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optName.value(),
                optTypeName.value(),
                optAssignedExpr
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

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
        {
            return diagnostics;
        }

        const auto optBody = diagnostics.Collect(ParseBlockStmt(parser, scope));
        if (!optBody.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::pair
            {
                optCondition.value(),
                optBody.value()
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

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
        {
            return diagnostics;
        }

        const auto optBody = diagnostics.Collect(ParseBlockStmt(parser, scope));
        if (!optBody.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::pair
            {
                optCondition.value(),
                optBody.value()
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

        const auto optBody = diagnostics.Collect(ParseBlockStmt(parser, scope));
        if (!optBody.has_value())
        {
            return diagnostics;
        }

        return Expected{ optBody.value(), diagnostics };
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

        const auto optIfBlock = diagnostics.Collect(ParseIfBlock(
            parser,
            scope
        ));
        if (!optIfBlock.has_value())
        {
            return diagnostics;
        }

        conditions.push_back(optIfBlock.value().first);
        bodies.push_back(optIfBlock.value().second);

        while (parser.Peek() == TokenKind::ElifKeyword)
        {
            const auto optElifBlock = diagnostics.Collect(ParseElifBlock(
                parser,
                scope
            ));
            if (!optElifBlock.has_value())
            {
                return diagnostics;
            }

            conditions.push_back(optElifBlock.value().first);
            bodies.push_back(optElifBlock.value().second);
        }

        if (parser.Peek() == TokenKind::ElseKeyword)
        {
            const auto optElseBlock = diagnostics.Collect(ParseElseBlock(
                parser,
                scope
            ));
            if (!optElseBlock.has_value())
            {
                return diagnostics;
            }

            bodies.push_back(optElseBlock.value());
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

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
        {
            return diagnostics;
        }

        const auto optBody = diagnostics.Collect(ParseBlockStmt(parser, scope));
        if (!optBody.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const WhileStmtNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optCondition.value(),
                optBody.value()
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
            optExpr = diagnostics.Collect(ParseExpr(parser, scope));
            if (!optExpr.has_value())
            {
                return diagnostics;
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

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
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
                optCondition.value()
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

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
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
                    optExpr.value()
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
    
        const auto optRhsExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optRhsExpr.has_value())
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
                    optExpr.value(),
                    optRhsExpr.value()
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
                    optExpr.value(),
                    optRhsExpr.value(),
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

            const auto optModifier = diagnostics.Collect(GetModifier(
                modifierToken
            ));
            if (!optModifier.has_value())
            {
                continue;
            }

            const auto allowedModifierIt = std::find(
                begin(allowedModifiers),
                end  (allowedModifiers),
                optModifier.value()
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

            modifierToTokenMap[optModifier.value()] = modifierToken;
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optNameToken = diagnostics.Collect(ParseFunctionOrOpNameToken(
            parser,
            scope
        ));
        if (!optNameToken.has_value())
        {
            return diagnostics;
        }

        const auto optParams = diagnostics.Collect(ParseParams(
            parser,
            selfScope
        ));
        if (!optParams.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        bool isExtern = false;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Self, Modifier::Extern },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            ));

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

        const auto optName = diagnostics.Collect(CreateFunctionOrOpName(
            optNameToken.value(),
            optParams.value().size(),
            accessModifier,
            optSelfToken.has_value()
        ));
        if (!optName.has_value())
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
            optBody = diagnostics.Collect(ParseBlockStmt(
                parser,
                selfScope
            ));
            if (!optBody.has_value())
            {
                return diagnostics;
            }
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
                optName.value(),
                optTypeName.value(),
                optAttributes.value(),
                accessModifier,
                optSelfParam,
                optParams.value(),
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        const auto optTemplateParams = diagnostics.Collect(ParseTemplateParams(
            parser,
            selfScope
        ));
        if (!optTemplateParams.has_value())
        {
            return diagnostics;
        }

        const auto optParams = diagnostics.Collect(ParseParams(parser, selfScope));
        if (!optParams.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Self },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            ));

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }

            if (modifierToTokenMap.contains(Modifier::Self))
            {
                optSelfToken = modifierToTokenMap.at(Modifier::Self);
            }
        }

        const auto optBody = diagnostics.Collect(ParseBlockStmt(
            parser,
            selfScope
        ));
        if (!optBody.has_value())
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
            optName.value(),
            optTypeName.value(),
            optAttributes.value(),
            accessModifier,
            selfParam,
            optParams.value(),
            optBody.value()
        );

        return Expected
        {
            std::make_shared<const FunctionTemplateNode>(
                function->GetSrcLocation(),
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                optTemplateParams.value(),
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

        const auto optTypeName = diagnostics.Collect(ParseSymbolName(
            parser,
            scope
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        // TODO: Remove this block after impl template specialization
        {
            const auto& sections = optTypeName.value().Sections;

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
                    optTypeName.value().Sections.front().Name.SrcLocation;
                const auto lastSectionSoruceLocation =
                    optTypeName.value().Sections.back().Name.SrcLocation;

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
                const auto optFunction = diagnostics.Collect(ParseImplFunction(
                    parser,
                    selfScope,
                    optTypeName.value()
                ));
                if (optFunction.has_value())
                {
                    functions.push_back(optFunction.value());
                    continue;
                }
            }
            else if (IsFunctionTemplateBegin(parser))
            {
                const auto optFunctionTemplate = diagnostics.Collect(ParseImplFunctionTemplate(
                    parser,
                    selfScope,
                    optTypeName.value()
                ));
                if (optFunctionTemplate.has_value())
                {
                    functionTemplates.push_back(optFunctionTemplate.value());
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
                optTypeName.value(),
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optNameToken = diagnostics.Collect(ParseFunctionOrOpNameToken(
            parser,
            scope
        ));
        if (!optNameToken.has_value())
        {
            return diagnostics;
        }

        const auto optTemplateParams = diagnostics.Collect(ParseOptionalTemplateParams(
            parser,
            selfScope
        ));
        if (!optTemplateParams.has_value())
        {
            return diagnostics;
        }

        const auto optParams = diagnostics.Collect(ParseParams(
            parser,
            selfScope
        ));
        if (!optParams.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Self },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            ));

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }

            if (modifierToTokenMap.contains(Modifier::Self))
            {
                optSelfToken = modifierToTokenMap.at(Modifier::Self);
            }
        }

        const auto optName = diagnostics.Collect(CreateFunctionOrOpName(
            optNameToken.value(),
            optParams.value().size(),
            accessModifier,
            optSelfToken.has_value()
        ));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        const auto optBody = diagnostics.Collect(ParseBlockStmt(parser, scope));
        if (!optBody.has_value())
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
            optName.value(),
            optTypeName.value(),
            optAttributes.value(),
            accessModifier,
            optSelfParam,
            optParams.value(),
            optBody.value()
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
                optTemplateParams.value(),
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

        const auto optTemplateParams = diagnostics.Collect(ParseImplTemplateParams(
            parser,
            selfScope
        ));
        if (!optTemplateParams.has_value())
        {
            return diagnostics;
        }

        const auto optTypeName = diagnostics.Collect(ParseSymbolName(
            parser,
            scope
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        // TODO: Remove this block after impl template specialization
        {
            const auto isNotSpecialized = diagnostics.Collect([&]() -> Expected<void>
            {
                const auto& templateParams = optTemplateParams.value();
                const auto& typeNameSections = optTypeName.value().Sections;

                const auto templatedSectionIt = std::find_if_not(
                    begin(typeNameSections),
                    end  (typeNameSections) - 1,
                    [](const SymbolNameSection& section)
                    {
                        return section.TemplateArgs.empty();
                    }
                );
                if (templatedSectionIt != (end(typeNameSections) - 1))
                {
                    return diagnostics;
                }

                const auto& templateArgs = typeNameSections.back().TemplateArgs;
                if (templateParams.size() != templateArgs.size())
                {
                    return diagnostics;
                }

                std::unordered_set<std::string> templateParamSet{};
                const auto redefinedParamIt = std::find_if(
                    begin(templateParams),
                    end  (templateParams),
                    [&](const std::shared_ptr<const ImplTemplateParamNode>& templateParam)
                    {
                        const std::string& templateParamName =
                            templateParam->GetName().String;

                        if (templateParamSet.contains(templateParamName))
                        {
                            return true;
                        }

                        templateParamSet.insert(templateParamName);
                        return false;
                    }
                );
                if (redefinedParamIt != end(templateParams))
                {
                    return diagnostics;
                }

                const auto invalidArgIt = std::find_if(
                    begin(templateArgs),
                    end  (templateArgs),
                    [&](const SymbolName& arg)
                    {
                        if (arg.Sections.size() != 1)
                        {
                            return true;
                        }

                        if (!arg.Sections.back().TemplateArgs.empty())
                        {
                            return true;
                        }
                            
                        const std::string& templateArgName =
                            arg.Sections.front().Name.String;

                        if (!templateParamSet.contains(templateArgName))
                        {
                            return true;
                        }

                        templateParamSet.erase(templateArgName);
                        return false;
                    }
                );
            
                return Void{};
            }());
            if (!isNotSpecialized)
            {
                const auto firstSectionSoruceLocation =
                    optTypeName.value().Sections.front().Name.SrcLocation;
                const auto lastSectionSoruceLocation =
                    optTypeName.value().Sections.back().Name.SrcLocation;

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
                const auto optFunctionTemplate = diagnostics.Collect(ParseTemplatedImplFunction(
                    parser,
                    selfScope,
                    optTypeName.value(),
                    optTemplateParams.value()
                ));
                if (optFunctionTemplate.has_value())
                {
                    functionTemplates.push_back(optFunctionTemplate.value());
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

        auto typeTemplateName = optTypeName.value();
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        const auto optParams = diagnostics.Collect(ParseParams(
            parser,
            selfScope
        ));
        if (!optParams.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            selfScope,
            RefParsingKind::Disallow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        bool isExtern = false;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public, Modifier::Extern },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            ));

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
            optBody = diagnostics.Collect(ParseBlockStmt(
                parser,
                selfScope
            ));
            if (!optBody.has_value())
            {
                return diagnostics;
            }
        }

        return Expected
        {
            std::make_shared<const FunctionNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                selfScope,
                optName.value(),
                optTypeName.value(),
                optAttributes.value(),
                accessModifier,
                std::nullopt,
                optParams.value(),
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        const auto optTemplateParams = diagnostics.Collect(ParseTemplateParams(
            parser,
            selfScope
        ));
        if (!optTemplateParams.has_value())
        {
            return diagnostics;
        }

        const auto optParams = diagnostics.Collect(ParseParams(
            parser,
            selfScope
        ));
        if (!optParams.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]()
                {
                    return
                        (parser.Peek() == TokenKind::OpenBrace) ||
                        (parser.Peek() == TokenKind::Semicolon);
                }
            ));
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto optBody = diagnostics.Collect(ParseBlockStmt(
            parser,
            selfScope
        ));
        if (!optBody.has_value())
        {
            return diagnostics;
        }

        const auto function = std::make_shared<const FunctionNode>(
            SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
            selfScope,
            optName.value(),
            optTypeName.value(),
            optAttributes.value(),
            accessModifier,
            std::nullopt,
            optParams.value(),
            optBody.value()
        );

        return Expected
        {
            std::make_shared<const FunctionTemplateNode>(
                function->GetSrcLocation(),
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                optTemplateParams.value(),
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::Semicolon; }
            ));

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
                optName.value(),
                optTypeName.value(),
                optAttributes.value(),
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
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

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Disallow
        ));
        if (!optTypeName.has_value())
        {
            return diagnostics;
        }

        auto accessModifier = AccessModifier::Private;
        if (parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::Comma; }
            ));

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
                optName.value(),
                optTypeName.value(),
                optAttributes.value(),
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
            
            const auto optVar = diagnostics.Collect(ParseMemberVar(
                parser,
                scope,
                vars.size()
            ));
            if (optVar.has_value())
            {
                vars.push_back(optVar.value());
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
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
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::OpenBrace; }
            ));

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }
        
        const auto optBody = diagnostics.Collect(ParseStructBody(
            parser,
            selfScope
        ));
        if (!optBody.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::make_shared<const StructTypeNode>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                selfScope,
                optName.value(),
                optAttributes.value(),
                accessModifier,
                optBody.value()
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

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return diagnostics;
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return diagnostics;
        }

        const auto optTemplateParams = diagnostics.Collect(ParseTemplateParams(
            parser,
            selfScope
        ));
        if (!optTemplateParams.has_value())
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
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::OpenBrace; }
            ));

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto optBody = diagnostics.Collect(ParseStructBody(
            parser,
            selfScope
        ));
        if (!optBody.has_value())
        {
            return diagnostics;
        }

        const auto type = std::make_shared<const StructTypeNode>(
            SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
            selfScope,
            optName.value(),
            optAttributes.value(),
            accessModifier,
            optBody.value()
        );

        return Expected
        {
            std::make_shared<const TypeTemplateNode>(
                type->GetSrcLocation(),
                optTemplateParams.value(),
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
            const auto optStructTemplate = diagnostics.Collect(ParseStructTemplate(
                parser,
                scope
            ));
            if (!optStructTemplate.has_value())
            {
                return diagnostics;
            }

            return Expected
            {
                optStructTemplate.value(),
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
            const auto optStruct = diagnostics.Collect(ParseStruct(
                parser,
                scope
            ));
            if (!optStruct.has_value())
            {
                return diagnostics;
            }

            return Expected{ optStruct.value(), diagnostics };
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

        const auto optName = diagnostics.Collect(ParseNestedName(
            parser,
            scope
        ));
        if (!optName.has_value())
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
            const auto modifierToTokenMap = diagnostics.Collect(ParseModifiersUntil(
                parser,
                scope,
                { Modifier::Public },
                [&]() { return parser.Peek() == TokenKind::OpenBrace; }
            ));

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        std::vector<std::shared_ptr<Scope>> scopes{};
        scopes.push_back(scope);
        std::transform(
            begin(optName.value()),
            end  (optName.value()),
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
                const auto optModule = diagnostics.Collect(ParseModule(
                    parser,
                    selfScope
                ));
                if (optModule.has_value())
                {
                    modules.push_back(optModule.value());
                    continue;
                }
            }
            else if (IsTypeBegin(parser))
            {
                const auto optType = diagnostics.Collect(ParseType(
                    parser,
                    selfScope
                ));
                if (optType.has_value())
                {
                    types.push_back(optType.value());
                    continue;
                }
            }
            else if (IsTypeTemplateBegin(parser))
            {
                const auto optTypeTemplate = diagnostics.Collect(ParseTypeTemplate(
                    parser,
                    selfScope
                ));
                if (optTypeTemplate.has_value())
                {
                    typeTemplates.push_back(optTypeTemplate.value());
                    continue;
                }
            }
            else if (IsImplBegin(parser))
            {
                const auto optImpl = diagnostics.Collect(ParseImpl(
                    parser,
                    selfScope
                ));
                if (optImpl.has_value())
                {
                    normalImpls.push_back(optImpl.value());
                    continue;
                }
            }
            else if (IsTemplatedImplBegin(parser))
            {
                const auto optTemplatedImpl = diagnostics.Collect(ParseTemplatedImpl(
                    parser,
                    selfScope
                ));
                if (optTemplatedImpl.has_value())
                {
                    templatedImpls.push_back(optTemplatedImpl.value());
                    continue;
                }
            }
            else if (IsFunctionBegin(parser))
            {
                const auto optFunction = diagnostics.Collect(ParseFunction(
                    parser,
                    selfScope
                ));
                if (optFunction.has_value())
                {
                    functions.push_back(optFunction.value());
                    continue;
                }
            }
            else if (IsFunctionTemplateBegin(parser))
            {
                const auto optFunctionTemplate = diagnostics.Collect(ParseFunctionTemplate(
                    parser,
                    selfScope
                ));
                if (optFunctionTemplate.has_value())
                {
                    functionTemplates.push_back(optFunctionTemplate.value());
                    continue;
                }
            }
            else if (IsVarBegin(parser))
            {
                const auto optVar = diagnostics.Collect(ParseVar(parser, selfScope));
                if (optVar.has_value())
                {
                    vars.push_back(optVar.value());
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
                optName.value(),
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

        const auto optModule = diagnostics.Collect(ParseModule(parser, scope));
        if (!optModule.has_value())
        {
            return diagnostics;
        }

        ACE_ASSERT(parser.IsEnd());

        return
        {
            optModule.value(),
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
            fileBuffer->GetCompilation()->GetGlobalScope()
        );
    }
}
