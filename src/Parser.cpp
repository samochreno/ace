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
#include "SpecialIdentifier.hpp"
#include "Name.hpp"
#include "Identifier.hpp"

namespace Ace
{
    enum class ReferenceParsingKind
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

    static auto IsCompoundAssignmentOperator(
        const TokenKind t_tokenKind
    ) -> bool
    {
        switch (t_tokenKind)
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

    static auto IsUserPrefixOperator(const TokenKind t_tokenKind) -> bool
    {
        switch (t_tokenKind)
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

    static auto IsPrefixOperator(const TokenKind t_tokenKind) -> bool
    {
        if (IsUserPrefixOperator(t_tokenKind))
        {
            return true;
        }

        switch (t_tokenKind)
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

    static auto IsPostfixOperator(const TokenKind t_tokenKind) -> bool
    {
        switch (t_tokenKind)
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

    static auto IsUserBinaryOperator(const TokenKind t_tokenKind) -> bool
    {
        switch (t_tokenKind)
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

    static auto IsBinaryOperator(const TokenKind t_tokenKind) -> bool
    {
        if (IsUserBinaryOperator(t_tokenKind))
        {
            return true;
        }

        switch (t_tokenKind)
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

    static auto IsUserOperator(const TokenKind t_tokenKind) -> bool
    {
        return
            IsUserPrefixOperator(t_tokenKind) ||
            IsUserBinaryOperator(t_tokenKind);
    }

    static constexpr size_t MaxBinaryOperatorPrecedence = 9;
    static auto GetBinaryOperatorPrecedence(
        const TokenKind t_operator
    ) -> size_t
    {
        if (
            (t_operator == TokenKind::Asterisk) ||
            (t_operator == TokenKind::Slash) ||
            (t_operator == TokenKind::Percent)
            )
        {
            return MaxBinaryOperatorPrecedence;
        }

        if (
            (t_operator == TokenKind::Plus) ||
            (t_operator == TokenKind::Minus)
            )
        {
            return 8;
        }

        if (
            (t_operator == TokenKind::LessThanLessThan) ||
            (t_operator == TokenKind::GreaterThanGreaterThan)
            )
        {
            return 7;
        }

        if (
            (t_operator == TokenKind::LessThan) ||
            (t_operator == TokenKind::LessThanEquals) ||
            (t_operator == TokenKind::GreaterThan) ||
            (t_operator == TokenKind::GreaterThanEquals)
            )
        {
            return 6;
        }

        if (
            (t_operator == TokenKind::EqualsEquals) ||
            (t_operator == TokenKind::ExclamationEquals)
            )
        {
            return 5;
        }

        if (t_operator == TokenKind::Ampersand)
        {
            return 4;
        }

        if (t_operator == TokenKind::Caret)
        {
            return 3;
        }

        if (t_operator == TokenKind::VerticalBar)
        {
            return 2;
        }

        if (t_operator == TokenKind::AmpersandAmpersand)
        {
            return 1;
        }

        if (t_operator == TokenKind::VerticalBarVerticalBar)
        {
            return 0;
        }

        ACE_UNREACHABLE();
    };

    static auto CreateCollapsedPrefixExpr(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_expr,
        const Operator& t_operator
    ) -> std::shared_ptr<const IExprNode>
    {
        switch (t_operator.TokenKind)
        {
            case TokenKind::Exclamation:
            {
                return std::make_shared<const LogicalNegationExprNode>(
                    t_sourceLocation,
                    t_expr
                );
            }

            case TokenKind::BoxKeyword:
            {
                return std::make_shared<const BoxExprNode>(
                    t_sourceLocation,
                    t_expr
                );
            }

            case TokenKind::UnboxKeyword:
            {
                return std::make_shared<const UnboxExprNode>(
                    t_sourceLocation,
                    t_expr
                );
            }

            default:
            {
                return std::make_shared<const UserUnaryExprNode>(
                    t_sourceLocation,
                    t_expr, 
                    t_operator
                );
            }
        }
    }

    static auto CreateCollapsedBinaryExpr(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_lhsExpr,
        const std::shared_ptr<const IExprNode>& t_rhsExpr,
        const Operator& t_operator
    ) -> std::shared_ptr<const IExprNode>
    {
        switch (t_operator.TokenKind)
        {
            case TokenKind::AmpersandAmpersand:
            {
                return std::make_shared<const AndExprNode>(
                    t_sourceLocation,
                    t_lhsExpr,
                    t_rhsExpr
                );
            }

            case TokenKind::VerticalBarVerticalBar:
            {
                return std::make_shared<const OrExprNode>(
                    t_sourceLocation,
                    t_lhsExpr,
                    t_rhsExpr
                );
            }
             
            default:
            {
                return std::make_shared<const UserBinaryExprNode>(
                    t_sourceLocation,
                    t_lhsExpr,
                    t_rhsExpr,
                    t_operator
                );
            }
        }
    }

    static auto CreateSourceLocationRange(
        const std::shared_ptr<const Token>& t_firstToken,
        const std::shared_ptr<const Token>& t_lastToken
    ) -> SourceLocation
    {
        return
        {
            t_firstToken->SourceLocation,
             t_lastToken->SourceLocation,
        };
    }

    static auto CreateSourceLocationRange(
        const std::shared_ptr<const INode>& t_firstToken,
        const std::shared_ptr<const INode>& t_lastToken
    ) -> SourceLocation
    {
        return
        {
            t_firstToken->GetSourceLocation(),
             t_lastToken->GetSourceLocation(),
        };
    }

    static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const AttributeNode>>
    {
        return {};
    }

    static auto GetOperatorFunctionName(
        const std::shared_ptr<const Token>& t_operatorToken,
        const size_t t_paramCount
    ) -> Expected<const char*>
    {
        DiagnosticBag diagnosticBag{};

        switch (t_operatorToken->Kind)
        {
            case TokenKind::Asterisk:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::Multiplication,
                    diagnosticBag,
                };
            }

            case TokenKind::Slash:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::Division,
                    diagnosticBag,
                };
            }

            case TokenKind::Percent:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::Remainder,
                    diagnosticBag,
                };
            }

            case TokenKind::Plus:
            {
                if (t_paramCount == 1)
                {
                    return
                    {
                        SpecialIdentifier::Operator::UnaryPlus,
                        diagnosticBag,
                    };
                }

                if (t_paramCount == 2)
                {
                    return
                    {
                        SpecialIdentifier::Operator::Addition,
                        diagnosticBag,
                    };
                }

                return diagnosticBag.Add(
                    CreateUnexpectedUnaryOrBinaryOperatorParamCountError(t_operatorToken)
                );
            }

            case TokenKind::Minus:
            {
                if (t_paramCount == 1)
                {
                    return
                    {
                        SpecialIdentifier::Operator::UnaryNegation,
                        diagnosticBag,
                    };
                }

                if (t_paramCount == 2)
                {
                    return
                    {
                        SpecialIdentifier::Operator::Subtraction,
                        diagnosticBag,
                    };
                }

                return diagnosticBag.Add(
                    CreateUnexpectedUnaryOrBinaryOperatorParamCountError(t_operatorToken)
                );
            }

            case TokenKind::LessThan:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::LessThan,
                    diagnosticBag,
                };
            }

            case TokenKind::GreaterThan:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::GreaterThan,
                    diagnosticBag,
                };
            }

            case TokenKind::LessThanEquals:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::LessThanEquals,
                    diagnosticBag,
                };
            }

            case TokenKind::GreaterThanEquals:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::GreaterThanEquals,
                    diagnosticBag,
                };
            }

            case TokenKind::GreaterThanGreaterThan:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::RightShift,
                    diagnosticBag,
                };
            }
            
            case TokenKind::LessThanLessThan:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::LeftShift,
                    diagnosticBag,
                };
            }

            case TokenKind::EqualsEquals:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::Equals,
                    diagnosticBag,
                };
            }

            case TokenKind::ExclamationEquals:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::NotEquals,
                    diagnosticBag,
                };
            }

            case TokenKind::Caret:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }
    
                return
                {
                    SpecialIdentifier::Operator::XOR,
                    diagnosticBag,
                };
            }

            case TokenKind::VerticalBar:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::OR,
                    diagnosticBag,
                };
            }

            case TokenKind::Ampersand:
            {
                if (t_paramCount != 2)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::AND,
                    diagnosticBag,
                };
            }
            
            case TokenKind::Tilde:
            {
                if (t_paramCount != 1)
                {
                    return diagnosticBag.Add(
                        CreateUnexpectedUnaryOperatorParamCountError(t_operatorToken)
                    );
                }

                return
                {
                    SpecialIdentifier::Operator::OneComplement,
                    diagnosticBag,
                };
            }
            
            case TokenKind::Identifier:
            {
                if (t_operatorToken->String == SpecialIdentifier::Copy)
                {
                    if (t_paramCount != 2)
                    {
                        return diagnosticBag.Add(
                            CreateUnexpectedBinaryOperatorParamCountError(t_operatorToken)
                        );
                    }

                    return
                    {
                        SpecialIdentifier::Operator::Copy,
                        diagnosticBag,
                    };
                }
                
                if (t_operatorToken->String  == SpecialIdentifier::Drop)
                {
                    if (t_paramCount != 1)
                    {
                        return diagnosticBag.Add(
                            CreateUnexpectedUnaryOperatorParamCountError(t_operatorToken)
                        );
                    }

                    return
                    {
                        SpecialIdentifier::Operator::Drop,
                        diagnosticBag,
                    };
                }

                return diagnosticBag.Add(
                    CreateUnknownIdentifierOperatorError(t_operatorToken)
                );
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    static auto CreateFunctionOrOperatorName(
        const std::shared_ptr<const Token>& t_nameToken,
        const size_t t_paramCount,
        const AccessModifier t_accessModifier,
        const bool t_hasSelfParam
    ) -> Expected<Identifier>
    {
        DiagnosticBag diagnosticBag{};

        if (t_nameToken == TokenKind::Identifier)
        {
            return 
            {
                Identifier
                {
                    t_nameToken->SourceLocation,
                    t_nameToken->String,
                },
                diagnosticBag,
            };
        }

        const auto expName = GetOperatorFunctionName(t_nameToken, t_paramCount);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (t_accessModifier != AccessModifier::Public)
        {
            return diagnosticBag.Add(CreateOperatorMustBePublicError(t_nameToken));
        }

        if (t_hasSelfParam)
        {
            return diagnosticBag.Add(CreateInstanceOperatorError(t_nameToken));
        }

        return Identifier
        {
            t_nameToken->SourceLocation,
            expName.Unwrap(),
        };
    }

    static auto CreateSelfParam(
        const std::optional<std::shared_ptr<const Token>>& optSelfToken,
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_selfTypeName
    ) -> std::optional<std::shared_ptr<const SelfParamVarNode>>
    {
        if (!optSelfToken.has_value())
        {
            return std::nullopt;
        }

        return std::make_shared<const SelfParamVarNode>(
            optSelfToken.value()->SourceLocation,
            t_scope,
            t_selfTypeName
        );
    }

    static auto GetLiteralKind(
        const std::shared_ptr<const Token>& t_token
    ) -> Expected<LiteralKind>
    {
        DiagnosticBag diagnosticBag{};

        switch (t_token->Kind)
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
                    t_token
                ));
            };
        }
    }

    static auto GetModifier(
        const std::shared_ptr<const Token>& t_token
    ) -> Expected<Modifier>
    {
        DiagnosticBag diagnosticBag{};

        switch (t_token->Kind)
        {
            case TokenKind::PublicKeyword:
            {
                return { Modifier::Public, diagnosticBag };
            }

            case TokenKind::ExternKeyword:
            {
                return { Modifier::Extern, diagnosticBag };
            }

            case TokenKind::Identifier:
            {
                if (t_token->String == SpecialIdentifier::Self)
                {
                    return { Modifier::Self, diagnosticBag };
                }
            }

            default:
            {
                return diagnosticBag.Add(CreateUnknownModifierError(
                    t_token
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
            const FileBuffer* const t_fileBuffer,
            const std::vector<std::shared_ptr<const Token>>& t_tokens
        ) : m_FileBuffer{ t_fileBuffer }
        {
            const auto firstLineIt = begin(t_fileBuffer->GetLines());
            const auto  lastLineIt =   end(t_fileBuffer->GetLines()) - 1;

            const auto& firstLine = *firstLineIt;
            const auto&  lastLine = * lastLineIt;

            const SourceLocation leadingSourceLocation
            {
                t_fileBuffer,
                begin(firstLine),
                begin(firstLine) + 1,
            };
            const SourceLocation trailingSourceLocation
            {
                t_fileBuffer,
                end(lastLine) - 1,
                end(lastLine),
            };

            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSourceLocation,
                TokenKind::Identifier,
                t_fileBuffer->GetCompilation()->Package.Name
            ));
            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSourceLocation,
                TokenKind::Colon
            ));
            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSourceLocation,
                TokenKind::ModuleKeyword
            ));
            m_Tokens.push_back(std::make_shared<const Token>(
                leadingSourceLocation,
                TokenKind::OpenBrace
            ));

            std::transform(
                begin(t_tokens),
                end  (t_tokens),
                back_inserter(m_Tokens),
                [](const std::shared_ptr<const Token>& t_token)
                {
                    return t_token;
                }
            );

            m_Tokens.insert(
                end(m_Tokens) - 1,
                std::make_shared<const Token>(
                    trailingSourceLocation,
                    TokenKind::CloseBrace
                )
            );

               m_Iterator = begin(m_Tokens);
            m_EndIterator = end  (m_Tokens);
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
            m_Iterator++;
            UpdateNestLevel();
            return PeekBack();
        }

        auto DiscardUntil(
            const DiscardKind t_kind,
            const TokenKind t_tokenKind
        ) -> void
        {
            DiscardUntil(t_kind, std::vector{ t_tokenKind });
        }
        auto DiscardUntil(
            const DiscardKind t_kind,
            const std::vector<TokenKind>& t_tokenKinds
        ) -> void
        {
            const auto startNestLevel = GetNestLevel();

            while (Peek() != TokenKind::EndOfFile)
            {
                if (GetNestLevel() != startNestLevel)
                {
                    Eat();
                    continue;
                }

                const auto matchingTokenKindIt = std::find_if(
                    begin(t_tokenKinds),
                    end  (t_tokenKinds),
                    [&](const TokenKind t_tokenKind)
                    {
                        return t_tokenKind == Peek();
                    }
                );
                if (matchingTokenKindIt == end(t_tokenKinds))
                {
                    Eat();
                    continue;
                }

                if (t_kind == DiscardKind::Inclusive)
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
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek() == TokenKind::CastKeyword) ||
            (t_parser.Peek() == TokenKind::AddressOfKeyword) ||
            (t_parser.Peek() == TokenKind::SizeOfKeyword) ||
            (t_parser.Peek() == TokenKind::DerefAsKeyword);
    }

    static auto IsExprExprStart(
        const Parser& t_parser
    ) -> bool
    {
        return t_parser.Peek() == TokenKind::OpenParen;
    }

    static auto IsStructConstructionExprStart(
        const Parser& t_parser
    ) -> bool
    {
        return t_parser.Peek() == SpecialIdentifier::New;
    }

    static auto IsLiteralExprStart(
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek() == TokenKind::Int8) ||
            (t_parser.Peek() == TokenKind::Int16) ||
            (t_parser.Peek() == TokenKind::Int32) ||
            (t_parser.Peek() == TokenKind::Int64) ||
            (t_parser.Peek() == TokenKind::UInt8) ||
            (t_parser.Peek() == TokenKind::UInt16) ||
            (t_parser.Peek() == TokenKind::UInt32) ||
            (t_parser.Peek() == TokenKind::UInt64) ||
            (t_parser.Peek() == TokenKind::Int) ||
            (t_parser.Peek() == TokenKind::Float32) ||
            (t_parser.Peek() == TokenKind::Float64) ||
            (t_parser.Peek() == TokenKind::String) ||
            (t_parser.Peek() == TokenKind::TrueKeyword) ||
            (t_parser.Peek() == TokenKind::FalseKeyword);
    }

    static auto IsLiteralSymbolExprStart(
        const Parser& t_parser
    ) -> bool
    {
        return t_parser.Peek() == TokenKind::Identifier;
    }

    static auto IsKeywordStmtStart(
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek() == TokenKind::IfKeyword) ||
            (t_parser.Peek() == TokenKind::WhileKeyword) ||
            (t_parser.Peek() == TokenKind::ReturnKeyword) ||
            (t_parser.Peek() == TokenKind::ExitKeyword) ||
            (t_parser.Peek() == TokenKind::AssertKeyword);
    }

    static auto IsBlockStmtStart(
        const Parser& t_parser
    ) -> bool
    {
        return t_parser.Peek() == TokenKind::OpenBrace;
    }

    static auto IsModuleStart(
        const Parser& t_parser
    ) -> bool
    {
        if (t_parser.Peek(0) != TokenKind::Identifier)
        {
            return false;
        }

        size_t i = 1;
        while (t_parser.Peek(i) == TokenKind::ColonColon)
        {
            i++;

            if (t_parser.Peek(i) != TokenKind::Identifier)
            {
                return false;
            }

            i++;
        }

        return
            (t_parser.Peek(i + 0) == TokenKind::Colon) &&
            (t_parser.Peek(i + 1) == TokenKind::ModuleKeyword);
    }

    static auto GetItemTemplateCloseBracketIndex(
        const Parser& t_parser
    ) -> std::optional<size_t>
    {
        if (t_parser.Peek(0) != TokenKind::Identifier)
        {
            return std::nullopt;
        }

        if (t_parser.Peek(1) != TokenKind::OpenBracket)
        {
            return std::nullopt;
        }

        size_t i = 2;
        while (
            (t_parser.Peek(i) != TokenKind::EndOfFile) &&
            (t_parser.Peek(i) != TokenKind::CloseBracket)
            )
        {
            if (i != 2)
            {
                if (t_parser.Peek(i) != TokenKind::Comma)
                {
                    return std::nullopt;
                }

                i++;
            }

            if (t_parser.Peek(i) != TokenKind::Identifier)
            {
                return std::nullopt;
            }

            i++;
        }

        return (t_parser.Peek(i) == TokenKind::CloseBracket) ?
            i : std::optional<size_t>{};
    }

    static auto IsStructStart(
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek(0) == TokenKind::Identifier) &&
            (t_parser.Peek(1) == TokenKind::Colon) &&
            (t_parser.Peek(2) == TokenKind::StructKeyword);
    }

    static auto IsTypeStart(
        const Parser& t_parser
    ) -> bool
    {
        return IsStructStart(t_parser);
    }

    static auto IsStructTemplateStart(
        const Parser& t_parser
    ) -> bool
    {
        const auto optCloseBracketIndex = GetItemTemplateCloseBracketIndex(
            t_parser
        );
        if (!optCloseBracketIndex.has_value())
        {
            return false;
        }

        auto i = optCloseBracketIndex.value() + 1;

        if (t_parser.Peek(i) != TokenKind::Colon)
        {
            return false;
        }

        i++;

        return t_parser.Peek(i) == TokenKind::StructKeyword;
    }

    static auto IsTypeTemplateStart(
        const Parser& t_parser
    ) -> bool
    {
        return IsStructTemplateStart(t_parser);
    }

    static auto IsFunctionStart(
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek(0) == TokenKind::OperatorKeyword) ||
            (
                (t_parser.Peek(0) == TokenKind::Identifier) &&
                (t_parser.Peek(1) == TokenKind::OpenParen)
            );
    }

    static auto IsFunctionTemplateStart(
        const Parser& t_parser
    ) -> bool
    {
        const auto optCloseBracketIndex = GetItemTemplateCloseBracketIndex(
            t_parser
        );
        if (!optCloseBracketIndex.has_value())
        {
            return false;
        }

        const auto i = optCloseBracketIndex.value() + 1;
        return t_parser.Peek(i) == TokenKind::OpenParen;
    }

    static auto IsImplStart(
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek(0) == TokenKind::ImplKeyword) &&
            (t_parser.Peek(1) != TokenKind::OpenBracket);
    }

    static auto IsTemplatedImplStart(
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek(0) == TokenKind::ImplKeyword) &&
            (t_parser.Peek(1) == TokenKind::OpenBracket);
    }

    static auto IsVarStart(
        const Parser& t_parser
    ) -> bool
    {
        return
            (t_parser.Peek(0) == TokenKind::Identifier) &&
            (t_parser.Peek(1) == TokenKind::Colon) &&
            (t_parser.Peek(2) != TokenKind::StructKeyword) &&
            (t_parser.Peek(2) != TokenKind::ModuleKeyword);
    }

    static auto ParseExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IExprNode>>;

    static auto ParseOptionalTemplateArgs(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<SymbolName>>;

    static auto ParseStructConstructionExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const StructConstructionExprNode>>;

    static auto ParseStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IStmtNode>>;

    static auto ParseName(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<Identifier>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::Identifier)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Identifier
            ));
        }

        const auto& nameToken = t_parser.Eat();

        return Expected
        {
            Identifier
            {
                nameToken->SourceLocation,
                nameToken->String,
            },
            diagnosticBag,
        };
    }

    static auto ParseNestedName(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<Identifier>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<Identifier> nestedName{};

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        nestedName.push_back(std::move(expName.Unwrap()));

        while (t_parser.Peek() == TokenKind::ColonColon)
        {
            t_parser.Eat();

            const auto expName = ParseName(t_parser, t_scope);
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<SymbolNameSection>
    {
        DiagnosticBag diagnosticBag{};

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateArgs = ParseOptionalTemplateArgs(
            t_parser,
            t_scope
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<SymbolName>
    {
        DiagnosticBag diagnosticBag{};

        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (t_parser.Peek() == TokenKind::ColonColon)
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            t_parser.Eat();
        }
        
        std::vector<SymbolNameSection> sections{};

        const auto expSection = ParseSymbolNameSection(t_parser, t_scope);
        diagnosticBag.Add(expSection);
        if (!expSection)
        {
            return diagnosticBag;
        }

        sections.push_back(std::move(expSection.Unwrap()));

        while (t_parser.Peek() == TokenKind::ColonColon)
        {
            t_parser.Eat();

            const auto expSection = ParseSymbolNameSection(t_parser, t_scope);
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope,
        const ReferenceParsingKind t_referenceParsingKind
    ) -> Expected<TypeName>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<TypeNameModifier> modifiers{};

        if (t_referenceParsingKind == ReferenceParsingKind::Allow)
        {
            if (t_parser.Peek() == TokenKind::Ampersand)
            {
                modifiers.push_back(TypeNameModifier::Reference);
                t_parser.Eat();
            }
        }

        while (true)
        {
            if (t_parser.Peek() == TokenKind::Asterisk)
            {
                modifiers.push_back(TypeNameModifier::StrongPointer);
                t_parser.Eat();
                continue;
            }

            if (t_parser.Peek() == TokenKind::Tilde)
            {
                modifiers.push_back(TypeNameModifier::WeakPointer);
                t_parser.Eat();
                continue;
            }

            break;
        }

        const auto expSymbolName = ParseSymbolName(t_parser, t_scope);
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<Identifier>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        t_parser.Eat();

        std::vector<Identifier> names{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBracket)
            )
        {
            if (!names.empty())
            {
                if (t_parser.Peek() == TokenKind::Comma)
                {
                    t_parser.Eat();
                }
                else
                {
                    diagnosticBag.Add(CreateMissingTokenError(
                        t_parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expName = ParseName(t_parser, t_scope);
            diagnosticBag.Add(expName);
            if (expName)
            {
                names.push_back(std::move(expName.Unwrap()));
                continue;
            }

            t_parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseBracket }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseBracket)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBracket
            ));
        }

        if (names.empty())
        {
            return diagnosticBag.Add(CreateEmptyTemplateParamsError(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack())
            ));
        }

        return Expected{ names, diagnosticBag };
    }

    static auto ParseImplTemplateParams(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<std::shared_ptr<const ImplTemplateParamNode>>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expNames = ParseTemplateParamNames(t_parser, t_scope);
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
            [&](const Identifier& t_name)
            {
                return std::make_shared<const ImplTemplateParamNode>(
                    t_name.SourceLocation,
                    t_scope,
                    t_name
                );
            }
        );

        return Expected{ std::move(params), diagnosticBag };
    }

    static auto ParseTemplateParams(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expNames = ParseTemplateParamNames(t_parser, t_scope);
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
            [&](const Identifier& t_name)
            {
                return std::make_shared<const NormalTemplateParamNode>(
                    t_name.SourceLocation,
                    t_scope,
                    t_name
                );
            }
        );

        return Expected{ std::move(params), diagnosticBag };
    }

    static auto ParseOptionalTemplateParams(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
    {
        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return Expected
            {
                std::vector<std::shared_ptr<const NormalTemplateParamNode>>{},
                DiagnosticBag{},
            };
        }

        return ParseTemplateParams(t_parser, t_scope);
    }

    static auto ParseTemplateArgs(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<SymbolName>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        t_parser.Eat();

        std::vector<SymbolName> args{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBracket)
            )
        {
            if (!args.empty())
            {
                if (t_parser.Peek() == TokenKind::Comma)
                {
                    t_parser.Eat();
                }
                else
                {
                    diagnosticBag.Add(CreateMissingTokenError(
                        t_parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expArg = ParseSymbolName(t_parser, t_scope);
            diagnosticBag.Add(expArg);
            if (expArg)
            {
                args.push_back(std::move(expArg.Unwrap()));
                continue;
            }

            t_parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseBracket }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseBracket)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBracket
            ));
        }

        if (args.empty())
        {
            return diagnosticBag.Add(CreateEmptyTemplateArgsError(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack())
            ));
        }

        return Expected{ std::move(args), diagnosticBag };
    }

    static auto ParseOptionalTemplateArgs(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<SymbolName>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return Expected{ std::vector<SymbolName>{}, diagnosticBag };
        }

        const auto expArgs = ParseTemplateArgs(t_parser, t_scope);
        diagnosticBag.Add(expArgs);
        if (!expArgs)
        {
            return diagnosticBag;
        }

        return Expected{ std::move(expArgs.Unwrap()), diagnosticBag };
    }

    static auto ParseAttribute(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const AttributeNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBracket
            );
        }

        t_parser.Eat();

        const auto expStructConstructionExpr = ParseStructConstructionExpr(
            t_parser,
            t_scope
        );
        diagnosticBag.Add(expStructConstructionExpr);
        if (!expStructConstructionExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseBracket)
        {
            return CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseBracket
            );
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const AttributeNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                expStructConstructionExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseAttributes(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<std::shared_ptr<const AttributeNode>>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<std::shared_ptr<const AttributeNode>> attributes{};
        while (t_parser.Peek() == TokenKind::OpenBracket)
        {
            const auto expAttribute = ParseAttribute(t_parser, t_scope);
            diagnosticBag.Add(expAttribute);
            if (expAttribute)
            {
                attributes.push_back(expAttribute.Unwrap());
                continue;
            }

            t_parser.DiscardUntil(
                DiscardKind::Inclusive,
                TokenKind::CloseBracket
            );
        }

        return Expected{ std::move(attributes), diagnosticBag };
    }

    static auto ParseParam(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope,
        const size_t t_index
    ) -> Expected<std::shared_ptr<const NormalParamVarNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            );
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Allow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const NormalParamVarNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                t_index
            ),
            diagnosticBag,
        };
    }

    static auto ParseParams(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalParamVarNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        t_parser.Eat();

        std::vector<std::shared_ptr<const NormalParamVarNode>> params{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseParen)
            )
        {
            if (!params.empty())
            {
                if (t_parser.Peek() == TokenKind::Comma)
                {
                    t_parser.Eat();
                }
                else
                {
                    diagnosticBag.Add(CreateMissingTokenError(
                        t_parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expParam = ParseParam(
                t_parser,
                t_scope,
                params.size()
            );
            diagnosticBag.Add(expParam);
            if (expParam)
            {
                params.push_back(expParam.Unwrap());
                continue;
            }

            t_parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseParen }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseParen)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseParen
            ));
        }

        return Expected{ std::move(params), diagnosticBag };
    }

    static auto ParseArgs(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<std::shared_ptr<const IExprNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        t_parser.Eat();

        std::vector<std::shared_ptr<const IExprNode>> args{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseParen)
            )
        {
            if (!args.empty())
            {
                if (t_parser.Peek() == TokenKind::Comma)
                {
                    t_parser.Eat();
                }
                else
                {
                    diagnosticBag.Add(CreateMissingTokenError(
                        t_parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }
            }

            const auto expArg = ParseExpr(t_parser, t_scope);
            diagnosticBag.Add(expArg);
            if (expArg)
            {
                args.push_back(expArg.Unwrap());
                continue;
            }

            t_parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseParen }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseParen)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseParen
            ));
        }

        return Expected{ std::move(args), diagnosticBag };
    }

    static auto ParseLiteralExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const LiteralExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& literalToken = t_parser.Eat();

        const auto expLiteralKind = GetLiteralKind(literalToken);
        diagnosticBag.Add(expLiteralKind);
        if (!expLiteralKind)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const LiteralExprNode>(
                literalToken->SourceLocation,
                t_scope,
                expLiteralKind.Unwrap(),
                literalToken->String
            ),
            diagnosticBag,
        };
    }

    static auto ParseLiteralSymbolExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const LiteralSymbolExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expName = ParseSymbolName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const LiteralSymbolExprNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expName.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseStructConstructionExprArgs(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<StructConstructionExprArg>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        t_parser.Eat();

        std::vector<StructConstructionExprArg> args{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (!args.empty())
            {
                if (t_parser.Peek() == TokenKind::Comma)
                {
                    t_parser.Eat();
                }
                else
                {
                    diagnosticBag.Add(CreateMissingTokenError(
                        t_parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }

                if (t_parser.Peek() == TokenKind::CloseBrace)
                {
                    break;
                }
            }

            const auto expName = ParseName(t_parser, t_scope);
            diagnosticBag.Add(expName);
            if (!expName)
            {
                return diagnosticBag;
            }

            std::optional<std::shared_ptr<const IExprNode>> optValue{};
            if (t_parser.Peek() == TokenKind::Colon)
            {
                t_parser.Eat();

                const auto expValue = ParseExpr(t_parser, t_scope);
                diagnosticBag.Add(expValue);
                if (!expValue)
                {
                    return diagnosticBag;
                }

                optValue = expValue.Unwrap();
            }
            
            args.push_back(StructConstructionExprArg{
                expName.Unwrap(),
                optValue
            });
        }

        if (t_parser.Peek() == TokenKind::CloseBrace)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected{ std::move(args), diagnosticBag };
    }

    static auto ParseStructConstructionExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const StructConstructionExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != SpecialIdentifier::New)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenExpectedNewError(
                t_parser.Peek()
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseSymbolName(t_parser, t_scope);
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto expArgs = ParseStructConstructionExprArgs(
            t_parser,
            t_scope
        );
        diagnosticBag.Add(expArgs);
        if (!expArgs)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const StructConstructionExprNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expTypeName.Unwrap(),
                std::move(expArgs.Unwrap())
            ),
            diagnosticBag,
        };
    }

    static auto ParseCastExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const CastExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::CastKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CastKeyword
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Allow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        t_parser.Eat();

        const auto expExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const CastExprNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                expTypeName.Unwrap(),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseAddressOfExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const AddressOfExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::AddressOfKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::AddressOfKeyword
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        t_parser.Eat();

        const auto expExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const AddressOfExprNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseSizeOfExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const SizeOfExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::SizeOfKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::SizeOfKeyword
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Allow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const SizeOfExprNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expTypeName.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseDerefAsExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const DerefAsExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::DerefAsKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::DerefAsKeyword
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::OpenBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBracket
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Allow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseBracket)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseBracket
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        t_parser.Eat();

        const auto expExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const DerefAsExprNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                expTypeName.Unwrap(),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseExprExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ExprExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::OpenParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenParen
            ));
        }

        t_parser.Eat();

        const auto expExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::CloseParen)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::CloseParen
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const ExprExprNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseKeywordExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        switch (t_parser.Peek()->Kind)
        {
            case TokenKind::CastKeyword:
            {
                return ParseCastExpr(t_parser, t_scope);
            }

            case TokenKind::AddressOfKeyword:
            {
                return ParseAddressOfExpr(t_parser, t_scope);
            }

            case TokenKind::SizeOfKeyword:
            {
                return ParseSizeOfExpr(t_parser, t_scope);
            }

            case TokenKind::DerefAsKeyword:
            {
                return ParseDerefAsExpr(t_parser, t_scope);
            }

            default:
            {
                return DiagnosticBag{}.Add(CreateUnexpectedTokenError(
                    t_parser.Peek()
                ));
            }
        }
    }

    static auto ParsePrimaryExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        if (IsKeywordExprStart(t_parser))
        {
            return ParseKeywordExpr(t_parser, t_scope);
        }

        if (IsExprExprStart(t_parser))
        {
            return ParseExprExpr(t_parser, t_scope);
        }

        if (IsStructConstructionExprStart(t_parser))
        {
            return ParseStructConstructionExpr(t_parser, t_scope);
        }

        if (IsLiteralExprStart(t_parser))
        {
            return ParseLiteralExpr(t_parser, t_scope);
        }

        if (IsLiteralSymbolExprStart(t_parser))
        {
            return ParseLiteralSymbolExpr(t_parser, t_scope);
        }

        return DiagnosticBag{}.Add(CreateUnexpectedTokenError(
            t_parser.Peek()
        ));
    }

    static auto ParseSecondaryExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expPrimaryExpr = ParsePrimaryExpr(t_parser, t_scope);
        diagnosticBag.Add(expPrimaryExpr);
        if (!expPrimaryExpr)
        {
            return diagnosticBag;
        }

        auto expr = expPrimaryExpr.Unwrap();

        while (
            (t_parser.Peek() == TokenKind::Dot) ||
            (t_parser.Peek() == TokenKind::OpenParen)
            )
        {
            if (t_parser.Peek() == TokenKind::Dot)
            {
                t_parser.Eat();

                const auto expName = ParseSymbolNameSection(t_parser, t_scope);
                diagnosticBag.Add(expName);
                if (!expName)
                {
                    return diagnosticBag;
                }

                expr = std::make_shared<const MemberAccessExprNode>(
                    CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                    expr,
                    expName.Unwrap()
                );
            }

            if (t_parser.Peek() == TokenKind::OpenParen)
            {
                const auto expArgs = ParseArgs(t_parser, t_scope);
                diagnosticBag.Add(expArgs);
                if (!expArgs)
                {
                    return diagnosticBag;
                }

                expr = std::make_shared<const FunctionCallExprNode>(
                    CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        std::vector<Operator> operators{};
        while (IsPrefixOperator(t_parser.Peek()->Kind))
        {
            const auto& operatorToken = t_parser.Eat();
            operators.push_back(Operator{
                operatorToken->SourceLocation,
                operatorToken->Kind
            });
        }

        const auto expSecondaryExpr = ParseSecondaryExpr(t_parser, t_scope);
        diagnosticBag.Add(expSecondaryExpr);
        if (!expSecondaryExpr)
        {
            return diagnosticBag;
        }

        auto expr = expSecondaryExpr.Unwrap();
        while (!operators.empty())
        {
            const auto& op3rator = operators.back();

            const SourceLocation sourceLocation
            {
                op3rator.SourceLocation,
                t_parser.PeekBack()->SourceLocation,
            };
            expr = CreateCollapsedPrefixExpr(
                sourceLocation,
                expr,
                op3rator
            );

            operators.pop_back();
        }

        return Expected
        {
            expr,
            diagnosticBag,
        };
    }

    static auto ParseExpr(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IExprNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expUnaryExpr = ParseUnaryExpr(t_parser, t_scope);
        diagnosticBag.Add(expUnaryExpr);
        if (!expUnaryExpr)
        {
            return diagnosticBag;
        }

        std::vector<std::shared_ptr<const IExprNode>> exprs{};
        std::vector<Operator> operators{};

        exprs.push_back(expUnaryExpr.Unwrap());
        
        while (IsBinaryOperator(t_parser.Peek()->Kind))
        {
            const auto& operatorToken = t_parser.Eat();
            operators.push_back(Operator{
                operatorToken->SourceLocation,
                operatorToken->Kind
            });

            const auto expUnaryExpr = ParseUnaryExpr(t_parser, t_scope);
            diagnosticBag.Add(expUnaryExpr);
            if (!expUnaryExpr)
            {
                return diagnosticBag;
            }

            exprs.push_back(expUnaryExpr.Unwrap());
        }

        if (operators.empty())
        {
            ACE_ASSERT(exprs.size() == 1);

            return Expected
            {
                exprs.front(),
                diagnosticBag,
            };
        }

        for (
            ssize_t precedenceLevel = MaxBinaryOperatorPrecedence;
            precedenceLevel >= 0;
            precedenceLevel--
            )
        {
            bool didCollapseAny = true;
            while (didCollapseAny)
            {
                didCollapseAny = false;

                for (size_t i = 0; i < operators.size(); i++)
                {
                    const auto op3rator = operators.at(i);
                    const auto precedence =
                        GetBinaryOperatorPrecedence(op3rator.TokenKind);

                    if (precedence == precedenceLevel)
                    {
                        didCollapseAny = true;

                        const auto lhsExpr = exprs.at(i);
                        const auto rhsExpr = exprs.at(i + 1);

                        operators.erase(begin(operators) + i);
                        exprs.erase(begin(exprs) + i + 1);
                        exprs.at(i) = CreateCollapsedBinaryExpr(
                            CreateSourceLocationRange(lhsExpr, rhsExpr),
                            lhsExpr,
                            rhsExpr,
                            op3rator
                        );

                        break;
                    }
            }
            }
            
        }

        ACE_ASSERT(exprs.size() == 1);
        ACE_ASSERT(operators.empty());

        return Expected
        {
            exprs.front(),
            diagnosticBag,
        };
    }

    static auto ParseBlockStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const BlockStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        if (t_parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        t_parser.Eat();

        std::vector<std::shared_ptr<const IStmtNode>> stmts{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBrace)
            )
        {
            const auto expStmt = ParseStmt(t_parser, selfScope);
            diagnosticBag.Add(expStmt);
            if (expStmt)
            {
                stmts.push_back(expStmt.Unwrap());
                continue;
            }

            t_parser.DiscardUntil(
                DiscardKind::Inclusive,
                TokenKind::Semicolon
            );
        }

        if (t_parser.Peek() == TokenKind::CloseBrace)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const BlockStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                selfScope,
                stmts
            ),
            diagnosticBag,
        };
    }

    static auto ParseExprStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ExprStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const ExprStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                expExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseAssignmentStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const NormalAssignmentStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expLhsExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expLhsExpr);
        if (!expLhsExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Equals)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Equals
            ));
        }

        t_parser.Eat();

        const auto expRhsExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const NormalAssignmentStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expLhsExpr.Unwrap(),
                expRhsExpr.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseCompoundAssignmentStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const CompoundAssignmentStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expLhsExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expLhsExpr);
        if (!expLhsExpr)
        {
            return diagnosticBag;
        }

        if (!IsCompoundAssignmentOperator(t_parser.Peek()->Kind))
        {
            return diagnosticBag.Add(
                CreateUnexpectedTokenExpectedCompoundAssignmentOperatorError(t_parser.Peek())
            );
        }

        t_parser.Eat();

        const Operator op3rator
        {
            t_parser.PeekBack()->SourceLocation,
            t_parser.PeekBack()->Kind
        };

        const auto expRhsExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const CompoundAssignmentStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expLhsExpr.Unwrap(),
                expRhsExpr.Unwrap(),
                op3rator
            ),
            diagnosticBag,
        };
    }

    static auto ParseVarStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const VarStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Allow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        std::optional<std::shared_ptr<const IExprNode>> optAssignment{};
        if (t_parser.Peek() == TokenKind::Equals)
        {
            t_parser.Eat();

            const auto expExpr = ParseExpr(t_parser, t_scope);
            diagnosticBag.Add(expExpr);
            if (!expExpr)
            {
                return diagnosticBag;
            }

            optAssignment = expExpr.Unwrap();
        }

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const VarStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                optAssignment
            ),
            diagnosticBag,
        };
    }

    static auto ParseIfBlock(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::IfKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::IfKeyword
            ));
        }

        t_parser.Eat();

        const auto expCondition = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        const auto expBody = ParseBlockStmt(t_parser, t_scope);
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::ElifKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::ElifKeyword
            ));
        }

        t_parser.Eat();

        const auto expCondition = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        const auto expBody = ParseBlockStmt(t_parser, t_scope);
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const BlockStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::ElseKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::ElseKeyword
            ));
        }

        t_parser.Eat();

        const auto expBody = ParseBlockStmt(t_parser, t_scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected{ expBody.Unwrap(), diagnosticBag };
    }

    static auto ParseIfStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IfStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        std::vector<std::shared_ptr<const IExprNode>> conditions{};
        std::vector<std::shared_ptr<const BlockStmtNode>> bodies{};

        const auto expIfBlock = ParseIfBlock(t_parser, t_scope);
        diagnosticBag.Add(expIfBlock);
        if (!expIfBlock)
        {
            return diagnosticBag;
        }

        conditions.push_back(expIfBlock.Unwrap().first);
        bodies.push_back(expIfBlock.Unwrap().second);

        while (t_parser.Peek() == TokenKind::ElifKeyword)
        {
            const auto expElifBlock = ParseElifBlock(t_parser, t_scope);
            diagnosticBag.Add(expElifBlock);
            if (!expElifBlock)
            {
                return diagnosticBag;
            }

            conditions.push_back(expElifBlock.Unwrap().first);
            bodies.push_back(expElifBlock.Unwrap().second);
        }

        if (t_parser.Peek() == TokenKind::ElseKeyword)
        {
            const auto expElseBlock = ParseElseBlock(t_parser, t_scope);
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
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                conditions,
                bodies
            ),
            diagnosticBag,
        };
    }

    static auto ParseWhileStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const WhileStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::WhileKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::WhileKeyword
            ));
        }

        t_parser.Eat();

        const auto expCondition = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        const auto expBody = ParseBlockStmt(t_parser, t_scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const WhileStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expCondition.Unwrap(),
                expBody.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseReturnStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ReturnStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::ReturnKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::ReturnKeyword
            ));
        }

        t_parser.Eat();

        std::optional<std::shared_ptr<const IExprNode>> optExpr{};
        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            const auto expExpr = ParseExpr(t_parser, t_scope);
            diagnosticBag.Add(expExpr);
            if (!expExpr)
            {
                return diagnosticBag;
            }

            optExpr = expExpr.Unwrap();
        }

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const ReturnStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                optExpr
            ),
            diagnosticBag,
        };
    }

    static auto ParseExitStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ExitStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::ExitKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::ExitKeyword
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const ExitStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope
            ),
            diagnosticBag,
        };
    }

    static auto ParseAssertStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const AssertStmtNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        if (t_parser.Peek() != TokenKind::AssertKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::AssertKeyword
            ));
        }

        t_parser.Eat();

        const auto expCondition = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expCondition);
        if (!expCondition)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const AssertStmtNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expCondition.Unwrap()
            ),
            diagnosticBag,
        };
    }

    static auto ParseKeywordStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IStmtNode>>
    {
        switch (t_parser.Peek()->Kind)
        {
            case TokenKind::IfKeyword:
            {
                return ParseIfStmt(t_parser, t_scope);
            }

            case TokenKind::WhileKeyword:
            {
                return ParseWhileStmt(t_parser, t_scope);
            }

            case TokenKind::ReturnKeyword:
            {
                return ParseReturnStmt(t_parser, t_scope);
            }

            case TokenKind::ExitKeyword:
            {
                return ParseExitStmt(t_parser, t_scope);
            }

            case TokenKind::AssertKeyword:
            {
                return ParseAssertStmt(t_parser, t_scope);
            }

            default:
            {
                DiagnosticBag diagnosticBag{};
                return diagnosticBag.Add(CreateUnexpectedTokenError(
                    t_parser.Peek()
                ));
            }
        }
    }

    static auto ParseStmt(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const IStmtNode>>
    {
        if (IsVarStart(t_parser))
        {
            return ParseVarStmt(t_parser, t_scope);
        }
        else if (IsBlockStmtStart(t_parser))
        {
            return ParseBlockStmt(t_parser, t_scope);
        }
        else if (IsKeywordStmtStart(t_parser))
        {
            return ParseKeywordStmt(t_parser, t_scope);
        }

        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expExpr);
        if (!expExpr)
        {
            return diagnosticBag;
        }

        const bool isSemicolon = t_parser.Peek() == TokenKind::Semicolon;
        const bool isAssignment = t_parser.Peek() == TokenKind::Equals;
        const bool isCompoundAssignment = IsCompoundAssignmentOperator(
            t_parser.Peek()->Kind
        );

        if (
            isSemicolon ||
            (!isAssignment && !isCompoundAssignment)
            )
        {
            if (isSemicolon)
            {
                t_parser.Eat();
            }
            else
            {
                diagnosticBag.Add(CreateMissingTokenError(
                    t_parser.PeekBack(),
                    TokenKind::Semicolon
                ));
            }

            return Expected
            {
                std::make_shared<const ExprStmtNode>(
                    CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                    expExpr.Unwrap()
                ),
                diagnosticBag,
            };
        }

        t_parser.Eat();
    
        const Operator op3rator
        {
            t_parser.PeekBack()->SourceLocation,
            t_parser.PeekBack()->Kind,
        };

        const auto expRhsExpr = ParseExpr(t_parser, t_scope);
        diagnosticBag.Add(expRhsExpr);
        if (!expRhsExpr)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() == TokenKind::Semicolon)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::Semicolon
            ));
        }

        if (isAssignment)
        {
            return Expected
            {
                std::make_shared<const NormalAssignmentStmtNode>(
                    CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                    t_scope,
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
                    CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                    t_scope,
                    expExpr.Unwrap(),
                    expRhsExpr.Unwrap(),
                    op3rator
                ),
                diagnosticBag,
            };
        }

        ACE_UNREACHABLE();
    }

    static auto ParseFunctionOrOperatorNameToken(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const Token>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() == TokenKind::Identifier)
        {
            return Expected{ t_parser.Eat(), diagnosticBag };
        }

        if (t_parser.Peek() != TokenKind::OperatorKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Identifier
            ));
        }

        t_parser.Eat();

        if (
            (t_parser.Peek() != TokenKind::Identifier) &&
            !IsUserOperator(t_parser.Peek()->Kind)
            )
        {
            return diagnosticBag.Add(
                CreateUnexpectedTokenExpectedOverloadableOperatorError(t_parser.Peek())
            );
        }

        return Expected{ t_parser.Eat(), diagnosticBag };
    }

    static auto ParseModifiersUntil(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope,
        std::vector<Modifier> t_allowedModifiers,
        const std::function<bool()>& t_predicate
    ) -> Diagnosed<std::map<Modifier, std::shared_ptr<const Token>>>
    {
        DiagnosticBag diagnosticBag{};

        ACE_ASSERT(t_parser.Peek() == TokenKind::MinusGreaterThan);
        const auto& minusGreaterThanToken = t_parser.Eat();

        std::map<Modifier, std::shared_ptr<const Token>> modifierToTokenMap{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            !t_predicate()
        )
        {
            const auto modifierToken = t_parser.Eat();

            const auto expModifier = GetModifier(modifierToken);
            diagnosticBag.Add(expModifier);
            if (!expModifier)
            {
                continue;
            }

            const auto allowedModifierIt = std::find(
                begin(t_allowedModifiers),
                end  (t_allowedModifiers),
                expModifier.Unwrap()
            );
            if (allowedModifierIt == end(t_allowedModifiers))
            {
                diagnosticBag.Add(CreateForbiddenModifierError(
                    modifierToken
                ));
                continue;
            }

            t_allowedModifiers.erase(
                begin(t_allowedModifiers),
                allowedModifierIt
            );

            modifierToTokenMap[expModifier.Unwrap()] = modifierToken;
        }

        if (t_parser.Peek().get() == minusGreaterThanToken.get())
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_selfTypeName
    ) -> Expected<std::shared_ptr<const FunctionNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expNameToken = ParseFunctionOrOperatorNameToken(
            t_parser,
            t_scope
        );
        diagnosticBag.Add(expNameToken);
        if (!expNameToken)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(t_parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Disallow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        bool isExtern = false;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public, Modifier::Self, Modifier::Extern },
                [&]()
                {
                    return
                        (t_parser.Peek() == TokenKind::OpenBrace) ||
                        (t_parser.Peek() == TokenKind::Semicolon);
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

        const auto expName = CreateFunctionOrOperatorName(
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
            if (t_parser.Peek() != TokenKind::Semicolon)
            {
                return diagnosticBag.Add(CreateUnexpectedTokenError(
                    t_parser.Peek(),
                    TokenKind::Semicolon
                ));
            }

            t_parser.Eat();
        }
        else
        {
            const auto expBody = ParseBlockStmt(t_parser, selfScope);
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
            t_selfTypeName
        );

        return Expected
        {
            std::make_shared<const FunctionNode>(
                CreateSourceLocationRange(firstToken, t_parser.Peek()),
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_selfTypeName
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseTemplateParams(
            t_parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(t_parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Disallow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public, Modifier::Self },
                [&]()
                {
                    return
                        (t_parser.Peek() == TokenKind::OpenBrace) ||
                        (t_parser.Peek() == TokenKind::Semicolon);
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

        const auto expBody = ParseBlockStmt(t_parser, selfScope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto selfParam = CreateSelfParam(
            optSelfToken,
            selfScope,
            t_selfTypeName
        );

        const auto function = std::make_shared<const FunctionNode>(
            CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
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
                function->GetSourceLocation(),
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                expTemplateParams.Unwrap(),
                function
            ),
            diagnosticBag,
        };
    }

    static auto ParseImpl(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ImplNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        if (t_parser.Peek() != TokenKind::ImplKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::ImplKeyword
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseSymbolName(t_parser, t_scope);
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        // TODO: Remove this block after impl template specialization.
        {
            const auto& sections = expTypeName.Unwrap().Sections;

            const auto sectionWithTemplateArgsIt = std::find_if_not(
                begin(sections),
                end  (sections),
                [](const SymbolNameSection& t_section)
                {
                    return t_section.TemplateArgs.empty();
                }
            );
            if (sectionWithTemplateArgsIt != end(sections))
            {
                const auto firstSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.front().Name.SourceLocation;
                const auto lastSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.back().Name.SourceLocation;

                const SourceLocation sourceLocation
                {
                    firstSectionSoruceLocation.Buffer,
                    firstSectionSoruceLocation.CharacterBeginIterator,
                     lastSectionSoruceLocation.CharacterEndIterator,
                };

                return diagnosticBag.Add(CreateTemplateSpecializationError(
                    sourceLocation
                ));
            }
        }

        if (t_parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        t_parser.Eat();

        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (IsFunctionStart(t_parser))
            {
                const auto expFunction = ParseImplFunction(
                    t_parser,
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
            else if (IsFunctionTemplateStart(t_parser))
            {
                const auto expFunctionTemplate = ParseImplFunctionTemplate(
                    t_parser,
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
                    t_parser.Peek()
                ));
            }

            t_parser.DiscardUntil(
                DiscardKind::Inclusive,
                { TokenKind::CloseBrace, TokenKind::Semicolon }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseBrace)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const ImplNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                selfScope,
                expTypeName.Unwrap(),
                functions,
                functionTemplates
            ),
            diagnosticBag,
        };
    }

    static auto ParseTemplatedImplFunction(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_selfTypeName,
        const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& t_implTemplateParams
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expNameToken = ParseFunctionOrOperatorNameToken(
            t_parser,
            t_scope
        );
        diagnosticBag.Add(expNameToken);
        if (!expNameToken)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseOptionalTemplateParams(
            t_parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(t_parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Disallow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public, Modifier::Self },
                [&]()
                {
                    return
                        (t_parser.Peek() == TokenKind::OpenBrace) ||
                        (t_parser.Peek() == TokenKind::Semicolon);
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

        const auto expName = CreateFunctionOrOperatorName(
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

        const auto expBody = ParseBlockStmt(t_parser, t_scope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto optSelfParam = CreateSelfParam(
            optSelfToken,
            selfScope,
            t_selfTypeName
        );

        const auto function = std::make_shared<const FunctionNode>(
            CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
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
            begin(t_implTemplateParams),
            end  (t_implTemplateParams),
            back_inserter(clonedImplTemplateParams),
            [&](const std::shared_ptr<const ImplTemplateParamNode>& t_implTemplateParam)
            {
                return t_implTemplateParam->CloneInScope(selfScope);
            }
        );

        return Expected
        {
            std::make_shared<const FunctionTemplateNode>(
                function->GetSourceLocation(),
                clonedImplTemplateParams,
                expTemplateParams.Unwrap(),
                function
            ),
            diagnosticBag,
        };
    }

    static auto ParseTemplatedImpl(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const TemplatedImplNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        if (t_parser.Peek() != TokenKind::ImplKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::ImplKeyword
            ));
        }

        t_parser.Eat();

        const auto expTemplateParams = ParseImplTemplateParams(
            t_parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expTypeName = ParseSymbolName(t_parser, t_scope);
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        // TODO: Remove this block after impl template specialization.
        {
            const auto expIsNotSpecialized = [&]() -> Expected<void>
            {
                const auto& templateParams = expTemplateParams.Unwrap();
                const auto& typeNameSections = expTypeName.Unwrap().Sections;

                const bool foundTemplatedSection = std::find_if(
                    begin(typeNameSections),
                    end  (typeNameSections) - 1,
                    [](const SymbolNameSection& t_section)
                    {
                        return t_section.TemplateArgs.empty();
                    }
                ) == end(typeNameSections);

                ACE_TRY_ASSERT(!foundTemplatedSection);

                const auto& templateArgs = typeNameSections.back().TemplateArgs;
                ACE_TRY_ASSERT(templateParams.size() == templateArgs.size());

                std::unordered_set<std::string> templateParamSet{};
                ACE_TRY_VOID(TransformExpectedVector(templateParams,
                [&](const std::shared_ptr<const ImplTemplateParamNode>& t_templateParam) -> Expected<void>
                {
                    const std::string& templateParamName =
                        t_templateParam->GetName().String;

                    ACE_TRY_ASSERT(!templateParamSet.contains(
                        templateParamName
                    ));
                    templateParamSet.insert(templateParamName);

                    return Void{};
                }));
                
                ACE_TRY_VOID(TransformExpectedVector(templateArgs,
                [&](const SymbolName& t_arg) -> Expected<void>
                {
                    ACE_TRY_ASSERT(t_arg.Sections.size() == 1);
                    ACE_TRY_ASSERT(t_arg.Sections.back().TemplateArgs.empty());

                    const std::string& templateArgName =
                        t_arg.Sections.front().Name.String;
                    ACE_TRY_ASSERT(templateParamSet.contains(templateArgName));
                    templateParamSet.erase(templateArgName);

                    return Void{};
                }));

                return Void{};
            }();
            if (!expIsNotSpecialized)
            {
                const auto firstSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.front().Name.SourceLocation;
                const auto lastSectionSoruceLocation =
                    expTypeName.Unwrap().Sections.back().Name.SourceLocation;

                const SourceLocation sourceLocation
                {
                    firstSectionSoruceLocation.Buffer,
                    firstSectionSoruceLocation.CharacterBeginIterator,
                     lastSectionSoruceLocation.CharacterEndIterator,
                };

                return diagnosticBag.Add(CreateTemplateSpecializationError(
                    sourceLocation
                ));
            }
        }

        if (t_parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        t_parser.Eat();

        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (IsFunctionStart(t_parser) || IsFunctionTemplateStart(t_parser))
            {
                const auto expFunctionTemplate = ParseTemplatedImplFunction(
                    t_parser,
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
                    t_parser.Peek()
                ));
            }
            
            t_parser.DiscardUntil(
                DiscardKind::Inclusive,
                { TokenKind::Semicolon, TokenKind::CloseBrace }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseBrace)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        auto typeTemplateName = expTypeName.Unwrap();
        auto& typeTemplateNameLastSection = typeTemplateName.Sections.back();
        typeTemplateNameLastSection.TemplateArgs.clear();
        typeTemplateNameLastSection.Name.String = SpecialIdentifier::CreateTemplate(
            typeTemplateNameLastSection.Name.String
        );

        return Expected
        {
            std::make_shared<const TemplatedImplNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                selfScope,
                typeTemplateName,
                std::vector<std::shared_ptr<const FunctionNode>>{},
                functionTemplates
            ),
            diagnosticBag,
        };
    }

    static auto ParseFunction(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const FunctionNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(t_parser, selfScope);
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            selfScope,
            ReferenceParsingKind::Disallow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto accessModifier = AccessModifier::Private;
        bool isExtern = false;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public, Modifier::Extern },
                [&]()
                {
                    return
                        (t_parser.Peek() == TokenKind::OpenBrace) ||
                        (t_parser.Peek() == TokenKind::Semicolon);
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
            if (t_parser.Peek() != TokenKind::Semicolon)
            {
                return diagnosticBag.Add(CreateUnexpectedTokenError(
                    t_parser.Peek(),
                    TokenKind::Semicolon
                ));
            }

            t_parser.Eat();
        }
        else
        {
            if (expName.Unwrap().String == "test_1")
            {
                [](){}();
            }

            const auto expBody = ParseBlockStmt(t_parser, selfScope);
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
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const FunctionTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseTemplateParams(
            t_parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        const auto expParams = ParseParams(
            t_parser,
            selfScope
        );
        diagnosticBag.Add(expParams);
        if (!expParams)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Disallow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto accessModifier = AccessModifier::Private;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public },
                [&]()
                {
                    return
                        (t_parser.Peek() == TokenKind::OpenBrace) ||
                        (t_parser.Peek() == TokenKind::Semicolon);
                }
            );
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto expBody = ParseBlockStmt(t_parser, selfScope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto function = std::make_shared<const FunctionNode>(
            CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
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
                function->GetSourceLocation(),
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                expTemplateParams.Unwrap(),
                function
            ),
            diagnosticBag,
        };
    }

    static auto ParseVar(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const StaticVarNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Disallow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto accessModifier = AccessModifier::Private;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public },
                [&]() { return t_parser.Peek() == TokenKind::Semicolon; }
            );
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();
             
            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        if (t_parser.Peek() != TokenKind::Semicolon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        t_parser.Eat();

        return Expected
        {
            std::make_shared<const StaticVarNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier
            ),
            diagnosticBag,
        };
    }

    static auto ParseMemberVar(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope,
        const size_t t_index
    ) -> Expected<std::shared_ptr<const InstanceVarNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        const auto expTypeName = ParseTypeName(
            t_parser,
            t_scope,
            ReferenceParsingKind::Disallow
        );
        diagnosticBag.Add(expTypeName);
        if (!expTypeName)
        {
            return diagnosticBag;
        }

        auto accessModifier = AccessModifier::Private;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public },
                [&]() { return t_parser.Peek() == TokenKind::Comma; }
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
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
                t_scope,
                expName.Unwrap(),
                expTypeName.Unwrap(),
                expAttributes.Unwrap(),
                accessModifier,
                t_index
            ),
            diagnosticBag,
        };
    }

    static auto ParseStructBody(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::vector<std::shared_ptr<const InstanceVarNode>>>
    {
        DiagnosticBag diagnosticBag{};

        if (t_parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        t_parser.Eat();

        std::vector<std::shared_ptr<const InstanceVarNode>> vars{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBrace)
            )
        {
            if (!vars.empty())
            {
                if (t_parser.Peek() == TokenKind::Comma)
                {
                    t_parser.Eat();
                }
                else
                {
                    diagnosticBag.Add(CreateMissingTokenError(
                        t_parser.PeekBack(),
                        TokenKind::Comma
                    ));
                }

                if (t_parser.Peek() == TokenKind::CloseBrace)
                {
                    break;
                }
            }
            
            const auto expVar = ParseMemberVar(
                t_parser,
                t_scope,
                vars.size()
            );
            diagnosticBag.Add(expVar);
            if (expVar)
            {
                vars.push_back(expVar.Unwrap());
                continue;
            }

            t_parser.DiscardUntil(
                DiscardKind::Exclusive,
                { TokenKind::Comma, TokenKind::CloseBrace }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseBrace)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected{ std::move(vars), diagnosticBag };
    }

    static auto ParseStruct(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const StructTypeNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::StructKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::StructKeyword
            ));
        }

        t_parser.Eat();

        auto accessModifier = AccessModifier::Private;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public },
                [&]() { return t_parser.Peek() == TokenKind::OpenBrace; }
            );
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }
        
        const auto expBody = ParseStructBody(t_parser, selfScope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        return Expected
        {
            std::make_shared<const StructTypeNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const TypeTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();
        const auto selfScope = t_scope->GetOrCreateChild({});

        const auto expAttributes = ParseAttributes(t_parser, t_scope);
        diagnosticBag.Add(expAttributes);
        if (!expAttributes)
        {
            return diagnosticBag;
        }

        const auto expName = ParseName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        const auto expTemplateParams = ParseTemplateParams(
            t_parser,
            selfScope
        );
        diagnosticBag.Add(expTemplateParams);
        if (!expTemplateParams)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::StructKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::StructKeyword
            ));
        }

        t_parser.Eat();

        auto accessModifier = AccessModifier::Private;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public },
                [&]() { return t_parser.Peek() == TokenKind::OpenBrace; }
            );
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        const auto expBody = ParseStructBody(t_parser, selfScope);
        diagnosticBag.Add(expBody);
        if (!expBody)
        {
            return diagnosticBag;
        }

        const auto type = std::make_shared<const StructTypeNode>(
            CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
            selfScope,
            expName.Unwrap(),
            expAttributes.Unwrap(),
            accessModifier,
            expBody.Unwrap()
        );

        return Expected
        {
            std::make_shared<const TypeTemplateNode>(
                type->GetSourceLocation(),
                expTemplateParams.Unwrap(),
                type
            ),
            diagnosticBag,
        };
    }

    static auto ParseTypeTemplate(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const TypeTemplateNode>>
    {
        DiagnosticBag diagnosticBag{};

        if (IsStructTemplateStart(t_parser))
        {
            const auto expStructTemplate = ParseStructTemplate(
                t_parser,
                t_scope
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

        return diagnosticBag.Add(CreateUnexpectedTokenError(t_parser.Peek()));
    }

    static auto ParseType(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ITypeNode>>
    {
        DiagnosticBag diagnosticBag{};

        if (IsStructStart(t_parser))
        {
            const auto expStruct = ParseStruct(t_parser, t_scope);
            diagnosticBag.Add(expStruct);
            if (!expStruct)
            {
                return expStruct;
            }

            return Expected{ expStruct.Unwrap(), diagnosticBag };
        }

        return diagnosticBag.Add(CreateUnexpectedTokenError(t_parser.Peek()));
    }

    static auto ParseModule(
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto& firstToken = t_parser.Peek();

        const auto expName = ParseNestedName(t_parser, t_scope);
        diagnosticBag.Add(expName);
        if (!expName)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::Colon)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::Colon
            ));
        }

        t_parser.Eat();

        if (t_parser.Peek() != TokenKind::ModuleKeyword)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::ModuleKeyword
            ));
        }

        t_parser.Eat();

        auto accessModifier = AccessModifier::Private;
        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            const auto dgnModifierToTokenMap = ParseModifiersUntil(
                t_parser,
                t_scope,
                { Modifier::Public },
                [&]() { return t_parser.Peek() == TokenKind::OpenBrace; }
            );
            diagnosticBag.Add(dgnModifierToTokenMap);

            const auto& modifierToTokenMap = dgnModifierToTokenMap.Unwrap();

            if (modifierToTokenMap.contains(Modifier::Public))
            {
                accessModifier = AccessModifier::Public;
            }
        }

        std::vector<std::shared_ptr<Scope>> scopes{};
        scopes.push_back(t_scope);
        std::transform(
            begin(expName.Unwrap()),
            end  (expName.Unwrap()),
            back_inserter(scopes),
            [&](const Identifier& t_name)
            {
                return scopes.back()->GetOrCreateChild(t_name.String);
            }
        );

        if (t_parser.Peek() != TokenKind::OpenBrace)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::OpenBrace
            ));
        }

        t_parser.Eat();

        std::vector<std::shared_ptr<const ModuleNode>> modules{};
        std::vector<std::shared_ptr<const ITypeNode>> types{};
        std::vector<std::shared_ptr<const TypeTemplateNode>> typeTemplates{};
        std::vector<std::shared_ptr<const ImplNode>> impls{};
        std::vector<std::shared_ptr<const TemplatedImplNode>> templatedImpls{};
        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        std::vector<std::shared_ptr<const StaticVarNode>> vars{};
        while (
            (t_parser.Peek() != TokenKind::EndOfFile) &&
            (t_parser.Peek() != TokenKind::CloseBrace)
            )
        {
            const auto selfScope = scopes.back();

            if (IsModuleStart(t_parser))
            {
                const auto expModule = ParseModule(t_parser, selfScope);
                diagnosticBag.Add(expModule);
                if (expModule)
                {
                    modules.push_back(expModule.Unwrap());
                    continue;
                }
            }
            else if (IsTypeStart(t_parser))
            {
                const auto expType = ParseType(t_parser, selfScope);
                diagnosticBag.Add(expType);
                if (expType)
                {
                    types.push_back(expType.Unwrap());
                    continue;
                }
            }
            else if (IsTypeTemplateStart(t_parser))
            {
                const auto expTypeTemplate = ParseTypeTemplate(
                    t_parser,
                    selfScope
                );
                diagnosticBag.Add(expTypeTemplate);
                if (expTypeTemplate)
                {
                    typeTemplates.push_back(expTypeTemplate.Unwrap());
                    continue;
                }
            }
            else if (IsImplStart(t_parser))
            {
                const auto expImpl = ParseImpl(
                    t_parser,
                    selfScope
                );
                diagnosticBag.Add(expImpl);
                if (expImpl)
                {
                    impls.push_back(expImpl.Unwrap());
                    continue;
                }
            }
            else if (IsTemplatedImplStart(t_parser))
            {
                const auto expTemplatedImpl = ParseTemplatedImpl(
                    t_parser,
                    selfScope
                );
                diagnosticBag.Add(expTemplatedImpl);
                if (expTemplatedImpl)
                {
                    templatedImpls.push_back(expTemplatedImpl.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionStart(t_parser))
            {
                const auto expFunction = ParseFunction(t_parser, selfScope);
                diagnosticBag.Add(expFunction);
                if (expFunction)
                {
                    functions.push_back(expFunction.Unwrap());
                    continue;
                }
            }
            else if (IsFunctionTemplateStart(t_parser))
            {
                const auto expFunctionTemplate = ParseFunctionTemplate(
                    t_parser,
                    selfScope
                );
                diagnosticBag.Add(expFunctionTemplate);
                if (expFunctionTemplate)
                {
                    functionTemplates.push_back(expFunctionTemplate.Unwrap());
                    continue;
                }
            }
            else if (IsVarStart(t_parser))
            {
                const auto expVar = ParseVar(t_parser, selfScope);
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
                    t_parser.Peek()
                ));
            }
            
            t_parser.DiscardUntil(
                DiscardKind::Inclusive,
                { TokenKind::Semicolon, TokenKind::CloseBrace }
            );
        }

        if (t_parser.Peek() == TokenKind::CloseBrace)
        {
            t_parser.Eat();
        }
        else
        {
            diagnosticBag.Add(CreateMissingTokenError(
                t_parser.PeekBack(),
                TokenKind::CloseBrace
            ));
        }

        return Expected
        {
            std::make_shared<const ModuleNode>(
                CreateSourceLocationRange(firstToken, t_parser.PeekBack()),
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
        Parser& t_parser,
        const std::shared_ptr<Scope>& t_scope
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expModule = ParseModule(t_parser, t_scope);
        diagnosticBag.Add(expModule);
        if (!expModule)
        {
            return diagnosticBag;
        }

        if (t_parser.Peek() != TokenKind::EndOfFile)
        {
            return diagnosticBag.Add(CreateUnexpectedTokenError(
                t_parser.Peek(),
                TokenKind::EndOfFile
            ));
        }

        t_parser.Eat();
        ACE_ASSERT(t_parser.IsEnd());

        return
        {
            expModule.Unwrap(),
            diagnosticBag,
        };
    }

    auto ParseAST(
        const FileBuffer* const t_fileBuffer,
        const std::vector<std::shared_ptr<const Token>>& t_tokens
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        Parser parser{ t_fileBuffer, t_tokens };
        return ParseAST(
            parser,
            t_fileBuffer->GetCompilation()->GlobalScope.Unwrap()
        );
    }
}
