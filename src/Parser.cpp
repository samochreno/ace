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
#include "Diagnostics/ParseDiagnostics.hpp"
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
        DiagnosticBag diagnosticBag{};

        switch (opToken->Kind)
        {
            case TokenKind::Asterisk:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Multiplication,
                    diagnosticBag,
                };
            }

            case TokenKind::Slash:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Division,
                    diagnosticBag,
                };
            }

            case TokenKind::Percent:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Remainder,
                    diagnosticBag,
                };
            }

            case TokenKind::Plus:
            {
                if (paramCount == 1)
                {
                    return
                    {
                        SpecialIdent::Op::UnaryPlus,
                        diagnosticBag,
                    };
                }

                if (paramCount == 2)
                {
                    return
                    {
                        SpecialIdent::Op::Addition,
                        diagnosticBag,
                    };
                }

                return diagnosticBag.Add(
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
                        diagnosticBag,
                    };
                }

                if (paramCount == 2)
                {
                    return
                    {
                        SpecialIdent::Op::Subtraction,
                        diagnosticBag,
                    };
                }

                return diagnosticBag.Add(
                    CreateUnexpectedUnaryOrBinaryOpParamCountError(opToken)
                );
            }

            case TokenKind::LessThan:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::LessThan,
                    diagnosticBag,
                };
            }

            case TokenKind::GreaterThan:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::GreaterThan,
                    diagnosticBag,
                };
            }

            case TokenKind::LessThanEquals:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::LessThanEquals,
                    diagnosticBag,
                };
            }

            case TokenKind::GreaterThanEquals:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::GreaterThanEquals,
                    diagnosticBag,
                };
            }

            case TokenKind::GreaterThanGreaterThan:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::RightShift,
                    diagnosticBag,
                };
            }
            
            case TokenKind::LessThanLessThan:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::LeftShift,
                    diagnosticBag,
                };
            }

            case TokenKind::EqualsEquals:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::Equals,
                    diagnosticBag,
                };
            }

            case TokenKind::ExclamationEquals:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::NotEquals,
                    diagnosticBag,
                };
            }

            case TokenKind::Caret:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }
    
                return
                {
                    SpecialIdent::Op::XOR,
                    diagnosticBag,
                };
            }

            case TokenKind::VerticalBar:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::OR,
                    diagnosticBag,
                };
            }

            case TokenKind::Ampersand:
            {
                if (paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::AND,
                    diagnosticBag,
                };
            }
            
            case TokenKind::Tilde:
            {
                if (paramCount != 1)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedUnaryOpParamCountError(opToken)
                    );
                }

                return
                {
                    SpecialIdent::Op::OneComplement,
                    diagnosticBag,
                };
            }
            
            case TokenKind::Ident:
            {
                if (opToken->String == SpecialIdent::Copy)
                {
                    if (paramCount != 2)
                    {
                        return diagnosticBag.Add(
                            CreateUnexpectedBinaryOpParamCountError(opToken)
                        );
                    }

                    return
                    {
                        SpecialIdent::Op::Copy,
                        diagnosticBag,
                    };
                }
                
                if (opToken->String == SpecialIdent::Drop)
                {
                    if (paramCount != 1)
                    {
                        return diagnosticBag.Add(
                            CreateUnexpectedUnaryOpParamCountError(opToken)
                        );
                    }

                    return
                    {
                        SpecialIdent::Op::Drop,
                        diagnosticBag,
                    };
                }

                return diagnosticBag.Add(
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
        DiagnosticBag diagnosticBag{};

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
                diagnosticBag,
            };
        }

        const auto expName = GetOpFunctionName(nameToken.Value, paramCount);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (accessModifier != AccessModifier::Public)
        {
            return diagnosticBag.Add(
                CreateOpMustBePublicError(nameToken.Value)
            );
        }

        if (hasSelfParam)
        {
            return diagnosticBag.Add(
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
        DiagnosticBag diagnosticBag{};

        switch (token->Kind)
        {
            case TokenKind::Int8:
            {
                return { LiteralKind::Int8, diagnosticBag };
            }

            case TokenKind::Int16:
            {
                return { LiteralKind::Int16, diagnosticBag };
            }

            case TokenKind::Int32:
            {
                return { LiteralKind::Int32, diagnosticBag };
            }

            case TokenKind::Int64:
            {
                return { LiteralKind::Int64, diagnosticBag };
            }

            case TokenKind::UInt8:
            {
                return { LiteralKind::UInt8, diagnosticBag };
            }

            case TokenKind::UInt16:
            {
                return { LiteralKind::UInt16, diagnosticBag };
            }

            case TokenKind::UInt32:
            {
                return { LiteralKind::UInt32, diagnosticBag };
            }

            case TokenKind::UInt64:
            {
                return { LiteralKind::UInt64, diagnosticBag };
            }

            case TokenKind::Int:
            {
                return { LiteralKind::Int, diagnosticBag };
            }

            case TokenKind::Float32:
            {
                return { LiteralKind::Float32, diagnosticBag };
            }

            case TokenKind::Float64:
            {
                return { LiteralKind::Float64, diagnosticBag };
            }

            case TokenKind::String:
            {
                return { LiteralKind::String, diagnosticBag };
            }

            case TokenKind::TrueKeyword:
            {
                return { LiteralKind::True, diagnosticBag };
            }

            case TokenKind::FalseKeyword:
            {
                return { LiteralKind::False, diagnosticBag };
            }

            default:
            {
                return diagnosticBag.Add(CreateUnexpectedTokenExpectedLiteralError(
                    token
                ));
            };
        }
    }

    static auto GetModifier(
        const std::shared_ptr<const Token>& token
    ) -> Expected<Modifier>
    {
        DiagnosticBag diagnosticBag{};

        switch (token->Kind)
        {
            case TokenKind::PublicKeyword:
            {
                return { Modifier::Public, diagnosticBag };
            }

            case TokenKind::ExternKeyword:
            {
                return { Modifier::Extern, diagnosticBag };
            }

            case TokenKind::Ident:
            {
                if (token->String == SpecialIdent::Self)
                {
                    return { Modifier::Self, diagnosticBag };
                }
            }

            default:
            {
                return diagnosticBag.Add(CreateUnknownModifierError(
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
        auto IsEnd() const -> bool
        {
            return m_Iterator == m_EndIterator;
        }
        auto Peek(const size_t distance = 0) const -> const std::shared_ptr<const Token>&
        {
            return *(m_Iterator + distance);
        }
        auto PeekBack() const -> const std::shared_ptr<const Token>&
        {
            return *(m_Iterator - 1);
        }

        auto Eat() -> const std::shared_ptr<const Token>&
        {
            ACE_ASSERT(m_Iterator <= m_EndIterator);
            m_Iterator++;
            UpdateNestLevel();
            return PeekBack();
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
            const auto startNestLevel = GetNestLevel();

            while (!IsEnd())
            {
                if (GetNestLevel() != startNestLevel)
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
    };

    static auto IsKeywordExprStart(
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

    static auto IsExprExprStart(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == TokenKind::OpenParen;
    }

    static auto IsStructConstructionExprStart(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == SpecialIdent::New;
    }

    static auto IsLiteralExprStart(
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

    static auto IsSymbolLiteralExprStart(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == TokenKind::Ident;
    }

    static auto IsKeywordStmtStart(
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

    static auto IsBlockStmtStart(
        const Parser& parser
    ) -> bool
    {
        return parser.Peek() == TokenKind::OpenBrace;
    }

    static auto IsModuleStart(
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

    static auto IsStructStart(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::Ident) &&
            (parser.Peek(1) == TokenKind::Colon) &&
            (parser.Peek(2) == TokenKind::StructKeyword);
    }

    static auto IsTypeStart(
        const Parser& parser
    ) -> bool
    {
        return IsStructStart(parser);
    }

    static auto IsStructTemplateStart(
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

    static auto IsTypeTemplateStart(
        const Parser& parser
    ) -> bool
    {
        return IsStructTemplateStart(parser);
    }

    static auto IsFunctionStart(
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

    static auto IsFunctionTemplateStart(
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

    static auto IsImplStart(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::ImplKeyword) &&
            (parser.Peek(1) != TokenKind::OpenBracket);
    }

    static auto IsTemplatedImplStart(
        const Parser& parser
    ) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::ImplKeyword) &&
            (parser.Peek(1) == TokenKind::OpenBracket);
    }

    static auto IsVarStart(
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
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::Ident)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag,
        };
    }

    static auto ParseNestedName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<Ident>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<Ident> nestedName{};

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        nestedName.push_back(std::move(expName.Unwrap()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto expName = ParseName(parser, scope);
            diagnosticBag.Add(expName);
            if (!expName)
            {
                return diagnosticBag;
            }

            nestedName.push_back(std::move(expName.Unwrap()));
        }

        return Expected{ std::move(nestedName), diagnosticBag };
    }

    static auto ParseSymbolNameSection(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolNameSection>
    {
        DiagnosticBag diagnosticBag{};

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateArgs = ParseOptionalTemplateArgs(
            parser,
            scope
        );
        diagnosticBag.Add(expTemplateArgs);
        if (!expTemplateArgs)
        {
            return diagnosticBag;
        }

        return Expected
        {
            SymbolNameSection
            {
                expName.Unwrap(),
                expTemplateArgs.Unwrap(),
            },
            diagnosticBag,
        };
    }

    static auto ParseSymbolName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolName>
    {
        DiagnosticBag diagnosticBag{};

        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (parser.Peek() == TokenKind::ColonColon)
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            parser.Eat();
        }
        
        std::vector<SymbolNameSection> sections{};

        const auto expSection = ParseSymbolNameSection(parser, scope);
        diagnosticBag.Add(expSection);
        if (!expSection)
        {
            return diagnosticBag;
        }

        sections.push_back(std::move(expSection.Unwrap()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto expSection = ParseSymbolNameSection(parser, scope);
            diagnosticBag.Add(expSection);
            if (!expSection)
            {
                return diagnosticBag;
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
            diagnosticBag,
        };
    }

    static auto ParseTypeName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const RefParsingKind refParsingKind
    ) -> Expected<TypeName>
    {
        DiagnosticBag diagnosticBag{};

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
        diagnosticBag.Add(expSymbolName);
        if (!expSymbolName)
        {
            return diagnosticBag;
        }

        return Expected
        {
            TypeName
            {
                expSymbolName.Unwrap(),
                modifiers,
            },
            diagnosticBag,
        };
    }

    static auto ParseTemplateParamNames(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<Ident>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
                    diagnosticBag.Add(CreateMissingTokenError(
                        parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expName = ParseName(parser, scope);
            diagnosticBag.Add(expName);
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseBracket
            ));
        }

        if (names.empty())
        {
            return diagnosticBag.Add(CreateEmptyTemplateParamsError(
                CreateSrcLocationRange(firstToken, parser.PeekBack())
            ));
        }

        return Expected{ names, diagnosticBag };
    }

    static auto ParseImplTemplateParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const ImplTemplateParamNode>>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expNames = ParseTemplateParamNames(parser, scope);
        diagnosticBag.Add(expNames);
        if (!expNames)
        {
            return diagnosticBag;
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

        return Expected{ std::move(params), diagnosticBag };
    }

    static auto ParseTemplateParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expNames = ParseTemplateParamNames(parser, scope);
        diagnosticBag.Add(expNames);
        if (!expNames)
        {
            return diagnosticBag;
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

        return Expected{ std::move(params), diagnosticBag };
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
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
                    diagnosticBag.Add(CreateMissingTokenError(
                        parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expArg = ParseSymbolName(parser, scope);
            diagnosticBag.Add(expArg);
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseBracket
            ));
        }

        if (args.empty())
        {
            return diagnosticBag.Add(CreateEmptyTemplateArgsError(
                CreateSrcLocationRange(firstToken, parser.PeekBack())
            ));
        }

        return Expected{ std::move(args), diagnosticBag };
    }

    static auto ParseOptionalTemplateArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return Expected{ std::vector<SymbolName>{}, diagnosticBag };
        }

        const auto expArgs = ParseTemplateArgs(parser, scope);
        diagnosticBag.Add(expArgs);
        if (!expArgs)
        {
            return diagnosticBag;
        }

        return Expected{ std::move(expArgs.Unwrap()), diagnosticBag };
    }

    static auto ParseAttribute(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AttributeNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

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
        diagnosticBag.Add(expStructConstructionExpr);
        if (!expStructConstructionExpr)
        {
            return diagnosticBag;
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
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                expStructConstructionExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseAttributes(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const AttributeNode>>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<std::shared_ptr<const AttributeNode>> attributes{};
        while (parser.Peek() == TokenKind::OpenBracket)
        {
            const auto expAttribute = ParseAttribute(parser, scope);
            diagnosticBag.Add(expAttribute);
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

        return Expected{ std::move(attributes), diagnosticBag };
    }

    static auto ParseParam(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const size_t index
    ) -> Expected<std::shared_ptr<const NormalParamVarNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const NormalParamVarNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                index
            ),
            diagnosticBag,
        };
    }

    static auto ParseParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalParamVarNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
                    diagnosticBag.Add(CreateMissingTokenError(
                        parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expParam = ParseParam(
                parser,
                scope,
                params.size()
            );
            diagnosticBag.Add(expParam);
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseParen
            ));
        }

        return Expected{ std::move(params), diagnosticBag };
    }

    static auto ParseArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const IExprNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
                    diagnosticBag.Add(CreateMissingTokenError(
                        parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expArg = ParseExpr(parser, scope);
            diagnosticBag.Add(expArg);
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseParen
            ));
        }

        return Expected{ std::move(args), diagnosticBag };
    }

    static auto ParseLiteralExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const LiteralExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& literalToken = parser.Eat();

        const auto expLiteralKind = GetLiteralKind(literalToken);
        diagnosticBag.Add(expLiteralKind);
        if (!expLiteralKind)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const LiteralExprNode>(
                literalToken->SrcLocation,
                scope,
                expLiteralKind.Unwrap(),
                literalToken->String
            ),
            diagnosticBag,
        };
    }

    static auto ParseSymbolLiteralExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SymbolLiteralExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expName = ParseSymbolName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const SymbolLiteralExprNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expName.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseStructConstructionExprArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<StructConstructionExprArg>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
                    diagnosticBag.Add(CreateMissingTokenError(
                        parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }

                if (parser.Peek() == TokenKind::CloseBrace)
                {
                    break;
                }
            }

            const auto expName = ParseName(parser, scope);
            diagnosticBag.Add(expName);
            if (!expName)
            {
                return diagnosticBag;
            }

            std::optional<std::shared_ptr<const IExprNode>> optValue{};
            if (parser.Peek() == TokenKind::Colon)
            {
                parser.Eat();

                const auto expValue = ParseExpr(parser, scope);
                diagnosticBag.Add(expValue);
                if (!expValue)
                {
                    return diagnosticBag;
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected{ std::move(args), diagnosticBag };
    }

    static auto ParseStructConstructionExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructConstructionExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != SpecialIdent::New)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenExpectedNewError(
                parser.Peek()
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseSymbolName(parser, scope);
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto expArgs = ParseStructConstructionExprArgs(
            parser,
            scope
        );
        diagnosticBag.Add(expArgs);
        if (!expArgs)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const StructConstructionExprNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expTypeName.Unwrap(),
                std::move(expArgs.Unwrap())
            ),
            diagnosticBag,
        };
    }

    static auto ParseCastExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const CastExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::CastKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CastKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const CastExprNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                expTypeName.Unwrap(),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseAddressOfExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AddressOfExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::AddressOfKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::AddressOfKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const AddressOfExprNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseSizeOfExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SizeOfExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::SizeOfKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::SizeOfKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const SizeOfExprNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expTypeName.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseDerefAsExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const DerefAsExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::DerefAsKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::DerefAsKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const DerefAsExprNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                expTypeName.Unwrap(),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseExprExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExprExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        parser.Eat();

        const auto expExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExprExprNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                expExpr.Unwrap()
            ),
            diagnosticBag,
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
        if (IsKeywordExprStart(parser))
        {
            return ParseKeywordExpr(parser, scope);
        }

        if (IsExprExprStart(parser))
        {
            return ParseExprExpr(parser, scope);
        }

        if (IsStructConstructionExprStart(parser))
        {
            return ParseStructConstructionExpr(parser, scope);
        }

        if (IsLiteralExprStart(parser))
        {
            return ParseLiteralExpr(parser, scope);
        }

        if (IsSymbolLiteralExprStart(parser))
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
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expPrimaryExpr = ParsePrimaryExpr(parser, scope);
        diagnosticBag.Add(expPrimaryExpr);
        if (!expPrimaryExpr)
        {
            return diagnosticBag;
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
                diagnosticBag.Add(expName);
                if (!expName)
                {
                    return diagnosticBag;
                }

                expr = std::make_shared<const MemberAccessExprNode>(
                    CreateSrcLocationRange(firstToken, parser.PeekBack()),
                    expr,
                    expName.Unwrap()
                );
            }

            if (parser.Peek() == TokenKind::OpenParen)
            {
                const auto expArgs = ParseArgs(parser, scope);
                diagnosticBag.Add(expArgs);
                if (!expArgs)
                {
                    return diagnosticBag;
                }

                expr = std::make_shared<const FunctionCallExprNode>(
                    CreateSrcLocationRange(firstToken, parser.PeekBack()),
                    expr,
                    expArgs.Unwrap()
                );
            }
        }

        return Expected
        {
            expr,
            diagnosticBag,
        };
    }

    static auto ParseUnaryExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnosticBag{};

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
        diagnosticBag.Add(expSecondaryExpr);
        if (!expSecondaryExpr)
        {
            return diagnosticBag;
        }

        auto expr = expSecondaryExpr.Unwrap();
        while (!ops.empty())
        {
            const auto& op = ops.back();

            const SrcLocation srcLocation
            {
                op.SrcLocation,
                parser.PeekBack()->SrcLocation,
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
            diagnosticBag,
        };
    }

    static auto ParseExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expUnaryExpr = ParseUnaryExpr(parser, scope);
        diagnosticBag.Add(expUnaryExpr);
        if (!expUnaryExpr)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(expUnaryExpr);
            if (!expUnaryExpr)
            {
                return diagnosticBag;
            }

            exprs.push_back(expUnaryExpr.Unwrap());
        }

        if (ops.empty())
        {
            ACE_ASSERT(exprs.size() == 1);

            return Expected
            {
                exprs.front(),
                diagnosticBag,
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
            diagnosticBag,
        };
    }

    static auto ParseBlockStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const BlockStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag.Add(expStmt);
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const BlockStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                selfScope,
                stmts
            ),
            diagnosticBag,
        };
    }

    static auto ParseExprStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExprStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExprStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseAssignmentStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const NormalAssignmentStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expLhsExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expLhsExpr);
        if (!expLhsExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Equals)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Equals
            ));
        }

        parser.Eat();

        const auto expRhsExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const NormalAssignmentStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expLhsExpr.Unwrap(),
                expRhsExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseCompoundAssignmentStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const CompoundAssignmentStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expLhsExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expLhsExpr);
        if (!expLhsExpr)
        {
            return diagnosticBag;
        }

        if (!IsCompoundAssignmentOp(parser.Peek()->Kind))
        {
            return diagnosticBag.Add(
                CreateUnexpectedTokenExpectedCompoundAssignmentOpError(parser.Peek())
            );
        }

        parser.Eat();

        const Op op
        {
            parser.PeekBack()->SrcLocation,
            parser.PeekBack()->Kind
        };

        const auto expRhsExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const CompoundAssignmentStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expLhsExpr.Unwrap(),
                expRhsExpr.Unwrap(),
                op
            ),
            diagnosticBag,
        };
    }

    static auto ParseVarStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const VarStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        std::optional<std::shared_ptr<const IExprNode>> optAssignment{};
        if (parser.Peek() == TokenKind::Equals)
        {
            parser.Eat();

            const auto expExpr = ParseExpr(parser, scope);
            diagnosticBag.Add(expExpr);
            if (!expExpr)
            {
                return diagnosticBag;
            }

            optAssignment = expExpr.Unwrap();
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const VarStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                optAssignment
            ),
            diagnosticBag,
        };
    }

    static auto ParseIfBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::IfKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::IfKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::pair
            {
                expCondition.Unwrap(),
                expBody.Unwrap()
            },
            diagnosticBag,
        };
    }

    static auto ParseElifBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::ElifKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ElifKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::pair
            {
                expCondition.Unwrap(),
                expBody.Unwrap()
            },
            diagnosticBag,
        };
    }

    static auto ParseElseBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const BlockStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::ElseKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ElseKeyword
            ));
        }

        parser.Eat();

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected{ expBody.Unwrap(), diagnosticBag };
    }

    static auto ParseIfStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IfStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        std::vector<std::shared_ptr<const IExprNode>> conditions{};
        std::vector<std::shared_ptr<const BlockStmtNode>> bodies{};

        const auto expIfBlock = ParseIfBlock(parser, scope);
        diagnosticBag.Add(expIfBlock);
        if (!expIfBlock)
        {
            return diagnosticBag;
        }

        conditions.push_back(expIfBlock.Unwrap().first);
        bodies.push_back(expIfBlock.Unwrap().second);

        while (parser.Peek() == TokenKind::ElifKeyword)
        {
            const auto expElifBlock = ParseElifBlock(parser, scope);
            diagnosticBag.Add(expElifBlock);
            if (!expElifBlock)
            {
                return diagnosticBag;
            }

            conditions.push_back(expElifBlock.Unwrap().first);
            bodies.push_back(expElifBlock.Unwrap().second);
        }

        if (parser.Peek() == TokenKind::ElseKeyword)
        {
            const auto expElseBlock = ParseElseBlock(parser, scope);
            diagnosticBag.Add(expElseBlock);
            if (!expElseBlock)
            {
                return diagnosticBag;
            }

            bodies.push_back(expElseBlock.Unwrap());
        }

        return Expected
        {
            std::make_shared<const IfStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                conditions,
                bodies
            ),
            diagnosticBag,
        };
    }

    static auto ParseWhileStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const WhileStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::WhileKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::WhileKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const WhileStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expCondition.Unwrap(),
                expBody.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseReturnStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ReturnStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::ReturnKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ReturnKeyword
            ));
        }

        parser.Eat();

        std::optional<std::shared_ptr<const IExprNode>> optExpr{};
        if (parser.Peek() != TokenKind::Semicolon)
        {
            const auto expExpr = ParseExpr(parser, scope);
            diagnosticBag.Add(expExpr);
            if (!expExpr)
            {
                return diagnosticBag;
            }

            optExpr = expExpr.Unwrap();
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ReturnStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                optExpr
            ),
            diagnosticBag,
        };
    }

    static auto ParseExitStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExitStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::ExitKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ExitKeyword
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExitStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope
            ),
            diagnosticBag,
        };
    }

    static auto ParseAssertStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AssertStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        if (parser.Peek() != TokenKind::AssertKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::AssertKeyword
            ));
        }

        parser.Eat();

        const auto expCondition = ParseExpr(parser, scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const AssertStmtNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expCondition.Unwrap()
            ),
            diagnosticBag,
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
                DiagnosticBag diagnosticBag{};
                return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        if (IsVarStart(parser))
        {
            return ParseVarStmt(parser, scope);
        }
        else if (IsBlockStmtStart(parser))
        {
            return ParseBlockStmt(parser, scope);
        }
        else if (IsKeywordStmtStart(parser))
        {
            return ParseKeywordStmt(parser, scope);
        }

        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
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
                diagnosticBag.Add(CreateMissingTokenError(
                    parser.PeekBack(),
                    TokenKind::Semicolon
                ));
            }

            return Expected
            {
                std::make_shared<const ExprStmtNode>(
                    CreateSrcLocationRange(firstToken, parser.PeekBack()),
                    expExpr.Unwrap()
                ),
                diagnosticBag,
            };
        }

        parser.Eat();
    
        const Op op
        {
            parser.PeekBack()->SrcLocation,
            parser.PeekBack()->Kind,
        };

        const auto expRhsExpr = ParseExpr(parser, scope);
        diagnosticBag.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnosticBag;
        }

        if (parser.Peek() == TokenKind::Semicolon)
        {
            parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::Semicolon
            ));
        }

        if (isAssignment)
        {
            return Expected
            {
                std::make_shared<const NormalAssignmentStmtNode>(
                    CreateSrcLocationRange(firstToken, parser.PeekBack()),
                    scope,
                    expExpr.Unwrap(),
                    expRhsExpr.Unwrap()
                ),
                diagnosticBag,
            };
        }

        if (isCompoundAssignment)
        {
            return Expected
            {
                std::make_shared<const CompoundAssignmentStmtNode>(
                    CreateSrcLocationRange(firstToken, parser.PeekBack()),
                    scope,
                    expExpr.Unwrap(),
                    expRhsExpr.Unwrap(),
                    op
                ),
                diagnosticBag,
            };
        }

        ACE_UNREACHABLE();
    }

    static auto ParseFunctionOrOpNameToken(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<FunctionOrOpNameToken>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() == TokenKind::Ident)
        {
            return Expected
            {
                FunctionOrOpNameToken
                {
                    FunctionOrOpNameKind::Function,
                    parser.Eat(),
                },
                diagnosticBag,
            };
        }

        if (parser.Peek() != TokenKind::OpKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            return diagnosticBag.Add(
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
            diagnosticBag,
        };
    }

    static auto ParseModifiersUntil(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        std::vector<Modifier> allowedModifiers,
        const std::function<bool()>& predicate
    ) -> Diagnosed<std::map<Modifier, std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(parser.Peek() == TokenKind::MinusGreaterThan);
        const auto& minusGreaterThanToken = parser.Eat();

        std::map<Modifier, std::shared_ptr<const Token>> modifierToTokenMap{};
        while (!parser.IsEnd() && !predicate())
        {
            const auto modifierToken = parser.Eat();

            const auto expModifier = GetModifier(modifierToken);
            diagnosticBag.Add(expModifier);
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
                diagnosticBag.Add(CreateForbiddenModifierError(
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
            diagnosticBag.Add(CreateEmptyModifiersError(
                minusGreaterThanToken
            ));
        }

        return Diagnosed
        {
            std::move(modifierToTokenMap),
            diagnosticBag,
        };
    }

    static auto ParseImplFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName
    ) -> Expected<std::shared_ptr<const FunctionNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expNameToken = ParseFunctionOrOpNameToken(
            parser,
            scope
        );
        diagnosticBag.Add(expNameToken);
        if (!expNameToken)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(dgnModifierToTokenMap);

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
                    diagnosticBag.Add(CreateExternInstanceFunctionError(
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
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        std::optional<std::shared_ptr<const BlockStmtNode>> optBody{};
        if (isExtern)
        {
            if (parser.Peek() != TokenKind::Semicolon)
            {
                return diagnosticBag.Add(CreateUnexpectedTokenError(
                    parser.Peek(),
                    TokenKind::Semicolon
                ));
            }

            parser.Eat();
        }
        else
        {
            const auto expBody = ParseBlockStmt(parser, selfScope);
            diagnosticBag.Add(expBody);
            if (!expBody)
            {
                return diagnosticBag;
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
                CreateSrcLocationRange(firstToken, parser.Peek()),
                selfScope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                optSelfParam,
                expParams.Unwrap(),
                optBody
            ),
            diagnosticBag,
        };
    }

    static auto ParseImplFunctionTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseTemplateParams(
            parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(dgnModifierToTokenMap);

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
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto selfParam = CreateSelfParam(
            optSelfToken,
            selfScope,
            selfTypeName
        );

        const auto function = std::make_shared<const FunctionNode>(
            CreateSrcLocationRange(firstToken, parser.PeekBack()),
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
            diagnosticBag,
        };
    }

    static auto ParseImpl(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ImplNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        if (parser.Peek() != TokenKind::ImplKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ImplKeyword
            ));
        }

        parser.Eat();

        const auto expTypeName = ParseSymbolName(parser, scope);
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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

                return diagnosticBag.Add(CreateTemplateSpecializationError(
                    srcLocation
                ));
            }
        }

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            if (IsFunctionStart(parser))
            {
                const auto expFunction = ParseImplFunction(
                    parser,
                    selfScope,
                    expTypeName.Unwrap()
                );
                diagnosticBag.Add(expFunction);
                if (expFunction)
                {
                    functions.push_back(expFunction.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionTemplateStart(parser))
            {
                const auto expFunctionTemplate = ParseImplFunctionTemplate(
                    parser,
                    selfScope,
                    expTypeName.Unwrap()
                );
                diagnosticBag.Add(expFunctionTemplate);
                if (expFunctionTemplate)
                {
                    functionTemplates.push_back(expFunctionTemplate.Unwrap());
                    continue;
                }
            }
            else
            {
                diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const ImplNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                selfScope,
                expTypeName.Unwrap(),
                functions,
                functionTemplates
            ),
            diagnosticBag,
        };
    }

    static auto ParseTemplatedImplFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName,
        const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& implTemplateParams
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expNameToken = ParseFunctionOrOpNameToken(
            parser,
            scope
        );
        diagnosticBag.Add(expNameToken);
        if (!expNameToken)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseOptionalTemplateParams(
            parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(dgnModifierToTokenMap);

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
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expBody = ParseBlockStmt(parser, scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto optSelfParam = CreateSelfParam(
            optSelfToken,
            selfScope,
            selfTypeName
        );

        const auto function = std::make_shared<const FunctionNode>(
            CreateSrcLocationRange(firstToken, parser.PeekBack()),
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
            diagnosticBag,
        };
    }

    static auto ParseTemplatedImpl(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TemplatedImplNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        if (parser.Peek() != TokenKind::ImplKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ImplKeyword
            ));
        }

        parser.Eat();

        const auto expTemplateParams = ParseImplTemplateParams(
            parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expTypeName = ParseSymbolName(parser, scope);
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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

                return diagnosticBag.Add(CreateTemplateSpecializationError(
                    srcLocation
                ));
            }
        }

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            if (IsFunctionStart(parser) || IsFunctionTemplateStart(parser))
            {
                const auto expFunctionTemplate = ParseTemplatedImplFunction(
                    parser,
                    selfScope,
                    expTypeName.Unwrap(),
                    expTemplateParams.Unwrap()
                );
                diagnosticBag.Add(expFunctionTemplate);
                if (expFunctionTemplate)
                {
                    functionTemplates.push_back(expFunctionTemplate.Unwrap());
                    continue;
                }
            }
            else
            {
                diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
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
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                selfScope,
                typeTemplateName,
                std::vector<std::shared_ptr<const FunctionNode>>{},
                functionTemplates
            ),
            diagnosticBag,
        };
    }

    static auto ParseFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const FunctionNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(dgnModifierToTokenMap);

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
                return diagnosticBag.Add(CreateUnexpectedTokenError(
                    parser.Peek(),
                    TokenKind::Semicolon
                ));
            }

            parser.Eat();
        }
        else
        {
            const auto expBody = ParseBlockStmt(parser, selfScope);
            diagnosticBag.Add(expBody);
            if (!expBody)
            {
                return diagnosticBag;
            }

            optBody = expBody.Unwrap();
        }

        return Expected
        {
            std::make_shared<const FunctionNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                selfScope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                std::nullopt,
                expParams.Unwrap(),
                optBody
            ),
            diagnosticBag,
        };
    }

    static auto ParseFunctionTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseTemplateParams(
            parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(
            parser,
            selfScope
        );
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto expBody = ParseBlockStmt(parser, selfScope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto function = std::make_shared<const FunctionNode>(
            CreateSrcLocationRange(firstToken, parser.PeekBack()),
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
            diagnosticBag,
        };
    }

    static auto ParseVar(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StaticVarNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const StaticVarNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier
            ),
            diagnosticBag,
        };
    }

    static auto ParseMemberVar(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const size_t index
    ) -> Expected<std::shared_ptr<const InstanceVarNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
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
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        return Expected
        {
            std::make_shared<const InstanceVarNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                index
            ),
            diagnosticBag,
        };
    }

    static auto ParseStructBody(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const InstanceVarNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
                    diagnosticBag.Add(CreateMissingTokenError(
                        parser.PeekBack(),
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
            diagnosticBag.Add(expVar);
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected{ std::move(vars), diagnosticBag };
    }

    static auto ParseStruct(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructTypeNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::StructKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }
        
        const auto expBody = ParseStructBody(parser, selfScope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const StructTypeNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                selfScope,
                expName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                expBody.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseStructTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TypeTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(parser, scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseTemplateParams(
            parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::StructKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto expBody = ParseStructBody(parser, selfScope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto type = std::make_shared<const StructTypeNode>(
            CreateSrcLocationRange(firstToken, parser.PeekBack()),
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
            diagnosticBag,
        };
    }

    static auto ParseTypeTemplate(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TypeTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        if (IsStructTemplateStart(parser))
        {
            const auto expStructTemplate = ParseStructTemplate(
                parser,
                scope
            );
            diagnosticBag.Add(expStructTemplate);
            if (!expStructTemplate)
            {
                return diagnosticBag;
            }

            return Expected
            {
                expStructTemplate.Unwrap(),
                diagnosticBag,
            };
        }

        return diagnosticBag.Add(CreateUnexpectedTokenError(parser.Peek()));
    }

    static auto ParseType(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ITypeNode>>
    {
        DiagnosticBag diagnosticBag{};

        if (IsStructStart(parser))
        {
            const auto expStruct = ParseStruct(parser, scope);
            diagnosticBag.Add(expStruct);
            if (!expStruct)
            {
                return expStruct;
            }

            return Expected{ expStruct.Unwrap(), diagnosticBag };
        }

        return diagnosticBag.Add(CreateUnexpectedTokenError(parser.Peek()));
    }

    static auto ParseModule(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = parser.Peek();

        const auto expName = ParseNestedName(parser, scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::ModuleKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag.Add(dgnModifierToTokenMap);

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
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        parser.Eat();

        std::vector<std::shared_ptr<const ModuleNode>> modules{};
        std::vector<std::shared_ptr<const ITypeNode>> types{};
        std::vector<std::shared_ptr<const TypeTemplateNode>> typeTemplates{};
        std::vector<std::shared_ptr<const ImplNode>> impls{};
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

            if (IsModuleStart(parser))
            {
                const auto expModule = ParseModule(parser, selfScope);
                diagnosticBag.Add(expModule);
                if (expModule)
                {
                    modules.push_back(expModule.Unwrap());
                    continue;
                }
            }
            else if (IsTypeStart(parser))
            {
                const auto expType = ParseType(parser, selfScope);
                diagnosticBag.Add(expType);
                if (expType)
                {
                    types.push_back(expType.Unwrap());
                    continue;
                }
            }
            else if (IsTypeTemplateStart(parser))
            {
                const auto expTypeTemplate = ParseTypeTemplate(
                    parser,
                    selfScope
                );
                diagnosticBag.Add(expTypeTemplate);
                if (expTypeTemplate)
                {
                    typeTemplates.push_back(expTypeTemplate.Unwrap());
                    continue;
                }
            }
            else if (IsImplStart(parser))
            {
                const auto expImpl = ParseImpl(
                    parser,
                    selfScope
                );
                diagnosticBag.Add(expImpl);
                if (expImpl)
                {
                    impls.push_back(expImpl.Unwrap());
                    continue;
                }
            }
            else if (IsTemplatedImplStart(parser))
            {
                const auto expTemplatedImpl = ParseTemplatedImpl(
                    parser,
                    selfScope
                );
                diagnosticBag.Add(expTemplatedImpl);
                if (expTemplatedImpl)
                {
                    templatedImpls.push_back(expTemplatedImpl.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionStart(parser))
            {
                const auto expFunction = ParseFunction(parser, selfScope);
                diagnosticBag.Add(expFunction);
                if (expFunction)
                {
                    functions.push_back(expFunction.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionTemplateStart(parser))
            {
                const auto expFunctionTemplate = ParseFunctionTemplate(
                    parser,
                    selfScope
                );
                diagnosticBag.Add(expFunctionTemplate);
                if (expFunctionTemplate)
                {
                    functionTemplates.push_back(expFunctionTemplate.Unwrap());
                    continue;
                }
            }
            else if (IsVarStart(parser))
            {
                const auto expVar = ParseVar(parser, selfScope);
                diagnosticBag.Add(expVar);
                if (expVar)
                {
                    vars.push_back(expVar.Unwrap());
                    continue;
                }
            }
            else
            {
                diagnosticBag.Add(CreateUnexpectedTokenError(
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
            diagnosticBag.Add(CreateMissingTokenError(
                parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const ModuleNode>(
                CreateSrcLocationRange(firstToken, parser.PeekBack()),
                scopes.front(),
                scopes.back(),
                expName.Unwrap(),
                accessModifier,
                modules,
                types,
                typeTemplates,
                impls,
                templatedImpls,
                functions,
                functionTemplates,
                vars
            ),
            diagnosticBag,
        };
    }

    static auto ParseAST(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expModule = ParseModule(parser, scope);
        diagnosticBag.Add(expModule);
        if (!expModule)
        {
            return diagnosticBag;
        }

        ACE_ASSERT(parser.IsEnd());

        return
        {
            expModule.Unwrap(),
            diagnosticBag,
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
