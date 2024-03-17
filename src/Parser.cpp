#include "Parser.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <set>
#include <unordered_set>
#include <map>

#include "Diagnostic.hpp"
#include "Diagnostics/ParsingDiagnostics.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include "Syntaxes/All.hpp"
#include "Compilation.hpp"
#include "FileBuffer.hpp"
#include "Scope.hpp"
#include "Assert.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdent.hpp"
#include "AnonymousIdent.hpp"
#include "Name.hpp"
#include "Ident.hpp"
#include "Keyword.hpp"

namespace Ace
{
    enum class RefParsingKind
    {
        Allow,
        Forbid,
    };

    enum class FunctionOrOpNameKind
    {
        Function,
        Op,
    };

    enum class SelfKind
    {
        Normal,
        StrongPtr,
    };

    struct FunctionOrOpNameToken
    {
        FunctionOrOpNameKind Kind{};
        Token Value{};
    };

    enum class Modifier
    {
        Pub,
        Extern,
        Self,
        StrongPtr,
    };

    struct NamedSymbolHeader
    {
        std::map<Modifier, Token> ModifierToTokenMap{};
        std::vector<std::shared_ptr<const AttributeSyntax>> Attributes{};
        Ident& Name;
        std::vector<Ident> NestedName{};
        std::shared_ptr<Scope> BodyScope{};
        std::vector<std::shared_ptr<const TypeParamSyntax>> TypeParams{};
        std::vector<std::shared_ptr<const NormalParamVarSyntax>> Params{};
    };

    enum class SymbolFlags
    {
        None           = 0,
        Generic        = 1 << 0,
        Paramized      = 1 << 2,
        NamedBodyScope = 1 << 3,
        NestedName     = 1 << 4,
        WhereClause    = 1 << 5,
    };

    inline auto operator&(const SymbolFlags lhs, const SymbolFlags rhs) -> bool
    {
        return (
            static_cast<std::underlying_type_t<SymbolFlags>>(lhs) &
            static_cast<std::underlying_type_t<SymbolFlags>>(rhs)
        ) != 0;
    }

    inline auto operator|(const SymbolFlags lhs, const SymbolFlags rhs) -> SymbolFlags
    {
        return static_cast<SymbolFlags>(
            static_cast<std::underlying_type_t<SymbolFlags>>(lhs) |
            static_cast<std::underlying_type_t<SymbolFlags>>(rhs)
        );
    }

    struct LocatedOp
    {
        SrcLocation SrcLocation{};
        TokenKind TokenKind{};
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
            case TokenKind::LockKeyword:
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

    static auto GetUnaryOp(const TokenKind op) -> Op
    {
        switch (op)
        {
            case TokenKind::Minus:
            {
                return Op::UnaryNegation;
            }

            case TokenKind::Tilde:
            {
                return Op::NOT;
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    static auto GetBinaryOp(const TokenKind op) -> Op
    {
        switch (op)
        {
            case TokenKind::Asterisk:
            case TokenKind::AsteriskEquals:
            {
                return Op::Multiplication;
            }

            case TokenKind::Slash:
            case TokenKind::SlashEquals:
            {
                return Op::Division;
            }

            case TokenKind::Percent:
            case TokenKind::PercentEquals:
            {
                return Op::Remainder;
            }

            case TokenKind::Plus:
            case TokenKind::PlusEquals:
            {
                return Op::Addition;
            }

            case TokenKind::Minus:
            case TokenKind::MinusEquals:
            {
                return Op::Subtraction;
            }

            case TokenKind::GreaterThanGreaterThan:
            case TokenKind::GreaterThanGreaterThanEquals:
            {
                return Op::RightShift;
            }

            case TokenKind::LessThanLessThan:
            case TokenKind::LessThanLessThanEquals:
            {
                return Op::LeftShift;
            }

            case TokenKind::LessThan:
            {
                return Op::LessThan;
            }

            case TokenKind::GreaterThan:
            {
                return Op::GreaterThan;
            }

            case TokenKind::LessThanEquals:
            {
                return Op::LessThanEquals;
            }

            case TokenKind::GreaterThanEquals:
            {
                return Op::GreaterThanEquals;
            }

            case TokenKind::EqualsEquals:
            {
                return Op::Equals;
            }

            case TokenKind::ExclamationEquals:
            {
                return Op::NotEquals;
            }

            case TokenKind::Ampersand:
            case TokenKind::AmpersandEquals:
            {
                return Op::AND;
            }

            case TokenKind::Caret:
            case TokenKind::CaretEquals:
            {
                return Op::XOR;
            }

            case TokenKind::VerticalBar:
            case TokenKind::VerticalBarEquals:
            {
                return Op::OR;
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    static auto CreateCollapsedPrefixExpr(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr,
        const LocatedOp& op
    ) -> std::shared_ptr<const IExprSyntax>
    {
        switch (op.TokenKind)
        {
            case TokenKind::Exclamation:
            {
                return std::make_shared<const LogicalNegationExprSyntax>(
                    srcLocation,
                    expr
                );
            }

            case TokenKind::LockKeyword:
            {
                return std::make_shared<const LockExprSyntax>(
                    srcLocation,
                    expr
                );
            }

            case TokenKind::BoxKeyword:
            {
                return std::make_shared<const BoxExprSyntax>(srcLocation, expr);
            }

            case TokenKind::UnboxKeyword:
            {
                return std::make_shared<const UnboxExprSyntax>(
                    srcLocation,
                    expr
                );
            }

            default:
            {
                return std::make_shared<const UserUnaryExprSyntax>(
                    srcLocation,
                    expr, 
                    op.SrcLocation,
                    GetUnaryOp(op.TokenKind)
                );
            }
        }
    }

    static auto CreateCollapsedBinaryExpr(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& lhsExpr,
        const std::shared_ptr<const IExprSyntax>& rhsExpr,
        const LocatedOp& op
    ) -> std::shared_ptr<const IExprSyntax>
    {
        switch (op.TokenKind)
        {
            case TokenKind::AmpersandAmpersand:
            {
                return std::make_shared<const AndExprSyntax>(
                    srcLocation,
                    lhsExpr,
                    rhsExpr
                );
            }

            case TokenKind::VerticalBarVerticalBar:
            {
                return std::make_shared<const OrExprSyntax>(
                    srcLocation,
                    lhsExpr,
                    rhsExpr
                );
            }
             
            default:
            {
                return std::make_shared<const UserBinaryExprSyntax>(
                    srcLocation,
                    lhsExpr,
                    rhsExpr,
                    op.SrcLocation,
                    GetBinaryOp(op.TokenKind)
                );
            }
        }
    }

    static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const AttributeSyntax>>
    {
        return {};
    }

    static auto CreateImplSelf(
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName
    ) -> std::shared_ptr<const ImplSelfSyntax>
    {
        return std::make_shared<const ImplSelfSyntax>(scope, selfTypeName);
    }

    static auto CreateSelfParamImpl(
        const NamedSymbolHeader& header,
        const bool isDyn
    ) -> Diagnosed<std::optional<std::shared_ptr<const SelfParamVarSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::optional<SelfKind> optSelfKind{};
        bool isSelfStrongPtr = false;
        std::vector<Token> selfTokens{};

        if (header.ModifierToTokenMap.contains(Modifier::StrongPtr))
        {
            const auto& strongPtrToken =
                header.ModifierToTokenMap.at(Modifier::StrongPtr);

            selfTokens.push_back(strongPtrToken);
            optSelfKind = SelfKind::StrongPtr;

            if (!header.ModifierToTokenMap.contains(Modifier::Self))
            {
                diagnostics.Add(CreateMissingSelfModifierAfterStrongPtrError(
                    strongPtrToken
                ));
            }
        }

        if (header.ModifierToTokenMap.contains(Modifier::Self))
        {
            const auto& selfToken =
                header.ModifierToTokenMap.at(Modifier::Self);

            selfTokens.push_back(selfToken);
            if (!optSelfKind.has_value())
            {
                optSelfKind = SelfKind::Normal;
            }
        }

        if (selfTokens.empty())
        {
            ACE_ASSERT(!optSelfKind.has_value());
            return Diagnosed{ std::nullopt, std::move(diagnostics) };
        }

        const SrcLocation srcLocation
        {
            selfTokens.front().SrcLocation,
            selfTokens.back ().SrcLocation,
        };

        std::vector<TypeNameModifier> typeNameModifiers
        {
            TypeNameModifier::Ref
        };

        switch (optSelfKind.value())
        {
            case SelfKind::Normal:
            {
                break;
            }

            case SelfKind::StrongPtr:
            {
                typeNameModifiers.push_back(isDyn ?
                    TypeNameModifier::DynStrongPtr :
                    TypeNameModifier::AutoStrongPtr
                );
                break;
            }
        }

        const SymbolName typeSymbolName
        {
            SymbolNameSection{ Ident{ srcLocation, SpecialIdent::SelfType } },
            SymbolNameResolutionScope::Local,
        };
        const TypeName typeName{ typeSymbolName, typeNameModifiers };

        return Diagnosed
        {
            std::make_shared<const SelfParamVarSyntax>(
                srcLocation,
                header.BodyScope,
                typeName
            ),
            std::move(diagnostics),
        };
    }

    static auto CreateSelfParam(
        const NamedSymbolHeader& header
    ) -> Diagnosed<std::optional<std::shared_ptr<const SelfParamVarSyntax>>>
    {
        return CreateSelfParamImpl(header, false);
    }

    static auto CreateDynSelfParam(
        const NamedSymbolHeader& header
    ) -> Diagnosed<std::optional<std::shared_ptr<const SelfParamVarSyntax>>>
    {
        return CreateSelfParamImpl(header, true);
    }

    static auto CreateName(const NamedSymbolHeader& header) -> SymbolName
    {
        std::vector<SymbolName> typeArgNames{};
        std::transform(
            begin(header.TypeParams),
            end  (header.TypeParams),
            back_inserter(typeArgNames),
            [](const std::shared_ptr<const TypeParamSyntax>& param)
            {
                return SymbolName
                {
                    SymbolNameSection{ param->GetName() },
                    SymbolNameResolutionScope::Local,
                };
            }
        );

        const SymbolName name
        {
            SymbolNameSection{ header.Name, typeArgNames },
            SymbolNameResolutionScope::Local,
        };

        return name;
    }

    static auto CloneTypeParamInScope(
        const std::shared_ptr<const TypeParamSyntax>& param,
        const std::shared_ptr<Scope>& scope
    ) -> std::shared_ptr<const TypeParamSyntax>
    {
        return std::make_shared<const TypeParamSyntax>(
            param->GetSrcLocation(),
            scope,
            param->GetName(),
            param->GetIndex()
        );
    }

    static auto GetLiteralKind(const Token& token) -> Expected<LiteralKind>
    {
        auto diagnostics = DiagnosticBag::Create();

        switch (token.Kind)
        {
            case TokenKind::Int8:
            {
                return Expected{ LiteralKind::Int8, std::move(diagnostics) };
            }

            case TokenKind::Int16:
            {
                return Expected{ LiteralKind::Int16, std::move(diagnostics) };
            }

            case TokenKind::Int32:
            {
                return Expected{ LiteralKind::Int32, std::move(diagnostics) };
            }

            case TokenKind::Int64:
            {
                return Expected{ LiteralKind::Int64, std::move(diagnostics) };
            }

            case TokenKind::UInt8:
            {
                return Expected{ LiteralKind::UInt8, std::move(diagnostics) };
            }

            case TokenKind::UInt16:
            {
                return Expected{ LiteralKind::UInt16, std::move(diagnostics) };
            }

            case TokenKind::UInt32:
            {
                return Expected{ LiteralKind::UInt32, std::move(diagnostics) };
            }

            case TokenKind::UInt64:
            {
                return Expected{ LiteralKind::UInt64, std::move(diagnostics) };
            }

            case TokenKind::Int:
            {
                return Expected{ LiteralKind::Int, std::move(diagnostics) };
            }

            case TokenKind::Float32:
            {
                return Expected{ LiteralKind::Float32, std::move(diagnostics) };
            }

            case TokenKind::Float64:
            {
                return Expected{ LiteralKind::Float64, std::move(diagnostics) };
            }

            case TokenKind::String:
            {
                return Expected{ LiteralKind::String, std::move(diagnostics) };
            }

            case TokenKind::TrueKeyword:
            {
                return Expected{ LiteralKind::True, std::move(diagnostics) };
            }

            case TokenKind::FalseKeyword:
            {
                return Expected{ LiteralKind::False, std::move(diagnostics) };
            }

            default:
            {
                diagnostics.Add(CreateUnexpectedTokenExpectedLiteralError(
                    token
                ));
                return std::move(diagnostics);
            };
        }
    }

    static auto GetModifier(const Token& token) -> Expected<Modifier>
    {
        auto diagnostics = DiagnosticBag::Create();

        switch (token.Kind)
        {
            case TokenKind::PubKeyword:
            {
                return Expected{ Modifier::Pub, std::move(diagnostics) };
            }

            case TokenKind::ExternKeyword:
            {
                return Expected{ Modifier::Extern, std::move(diagnostics) };
            }

            case TokenKind::SelfKeyword:
            {
                return Expected{ Modifier::Self, std::move(diagnostics) };
            }

            case TokenKind::Asterisk:
            {
                return Expected{ Modifier::StrongPtr, std::move(diagnostics) };
            }

            default:
            {
                diagnostics.Add(CreateUnknownModifierError(token));
                return std::move(diagnostics);
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
            std::vector<Token> tokens
        ) : m_FileBuffer{ fileBuffer },
            m_Tokens{ std::move(tokens) },
            m_Iterator{ begin(m_Tokens) },
            m_EndIterator{ end(m_Tokens) - 1 }
        {
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
            return Peek().SrcLocation;
        }
        auto GetLastSrcLocation() const -> const SrcLocation&
        {
            return m_LastSrcLocation;
        }
        auto IsEnd() const -> bool
        {
            return m_Iterator == m_EndIterator;
        }
        auto Peek(const size_t distance = 0) const -> const Token&
        {
            return *(m_Iterator + distance);
        }

        auto Eat() -> const Token&
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

            if (*(m_Iterator - 1) == TokenKind::OpenBrace)
            {
                m_NestLevel++;
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
        }

        const FileBuffer* m_FileBuffer{};
        std::vector<Token> m_Tokens{};
        std::vector<Token>::const_iterator m_Iterator{};
        std::vector<Token>::const_iterator m_EndIterator{};
        size_t m_NestLevel{};
        SrcLocation m_LastSrcLocation{};
    };

    static auto IsKeywordExprBegin(const Parser& parser) -> bool
    {
        switch (parser.Peek().Kind)
        {
            case TokenKind::CastKeyword:
            case TokenKind::AddressOfKeyword:
            case TokenKind::SizeOfKeyword:
            case TokenKind::DerefAsKeyword:
            case TokenKind::TypeInfoPtrKeyword:
            case TokenKind::VtblPtrKeyword:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsExprExprBegin(const Parser& parser) -> bool
    {
        return parser.Peek() == TokenKind::OpenParen;
    }

    static auto IsStructConstructionExprBegin(const Parser& parser) -> bool
    {
        return parser.Peek() == SpecialIdent::New;
    }

    static auto IsLiteralExprBegin(const Parser& parser) -> bool
    {
        switch (parser.Peek().Kind)
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

    static auto IsSymbolLiteralExprBegin(const Parser& parser) -> bool
    {
        return
            (parser.Peek() == TokenKind::Ident) ||
            (parser.Peek() == TokenKind::SelfKeyword);
    }

    static auto IsKeywordStmtBegin(const Parser& parser) -> bool
    {
        switch (parser.Peek().Kind)
        {
            case TokenKind::IfKeyword:
            case TokenKind::WhileKeyword:
            case TokenKind::RetKeyword:
            case TokenKind::ExitKeyword:
            case TokenKind::AssertKeyword:
            case TokenKind::CopyKeyword:
            case TokenKind::DropKeyword:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    static auto IsBlockStmtBegin(const Parser& parser) -> bool
    {
        return parser.Peek() == TokenKind::OpenBrace;
    }

    static auto GetNamedSymbolHeaderEndIndex(
        const Parser& parser,
        const SymbolFlags flags = SymbolFlags::None
    ) -> std::optional<size_t>
    {
        size_t i = 0;
        while (
            (parser.Peek(i) != TokenKind::EndOfFile) &&
            (parser.Peek(i) != TokenKind::ColonColon) &&
            (parser.Peek(i) != TokenKind::OpenBracket) &&
            (parser.Peek(i) != TokenKind::OpenParen) &&
            (parser.Peek(i) != TokenKind::Ident) 
            )
        {
            i++;
        }

        if (i != 0)
        {
            if (parser.Peek(i) != TokenKind::ColonColon)
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

        if (flags & SymbolFlags::NestedName)
        {
            while (parser.Peek(i) == TokenKind::ColonColon)
            {
                i++;

                if (parser.Peek(i) != TokenKind::Ident)
                {
                    return std::nullopt;
                }

                i++;
            }
        }

        if (
            (flags & SymbolFlags::Generic) &&
            (parser.Peek(i) == TokenKind::OpenBracket)
            )
        {
            i++;

            while (
                (parser.Peek(i) != TokenKind::EndOfFile) &&
                (parser.Peek(i) != TokenKind::CloseBracket)
                )
            {
                i++;
            }

            if (parser.Peek(i) != TokenKind::CloseBracket)
            {
                return std::nullopt;
            }

            i++;
        }

        if (flags & SymbolFlags::Paramized)
        {
            if (parser.Peek(i) != TokenKind::OpenParen)
            {
                return std::nullopt;
            }
            
            i++;

            while (
                (parser.Peek(i) != TokenKind::EndOfFile) &&
                (parser.Peek(i) != TokenKind::CloseParen)
                )
            {
                i++;
            }

            if (parser.Peek(i) != TokenKind::CloseParen)
            {
                return std::nullopt;
            }

            i++;
        }

        if (parser.Peek(i) != TokenKind::Colon)
        {
            return std::nullopt;
        }

        i++;

        return i;
    }

    static auto IsModBegin(const Parser& parser) -> bool
    {
        const auto optHeaderEndIndex = GetNamedSymbolHeaderEndIndex(
            parser,
            SymbolFlags::NestedName | SymbolFlags::NamedBodyScope
        );
        if (!optHeaderEndIndex.has_value())
        {
            return false;
        }

        const auto i = optHeaderEndIndex.value();

        return parser.Peek(i) == TokenKind::ModKeyword;
    }

    static auto IsTraitBegin(const Parser& parser) -> bool
    {
        const auto optHeaderEndIndex = GetNamedSymbolHeaderEndIndex(
            parser,
            SymbolFlags::Generic
        );
        if (!optHeaderEndIndex.has_value())
        {
            return false;
        }

        const auto i = optHeaderEndIndex.value();

        return parser.Peek(i) == TokenKind::TraitKeyword;
    }

    static auto IsStructBegin(const Parser& parser) -> bool
    {
        const auto optHeaderEndIndex = GetNamedSymbolHeaderEndIndex(
            parser,
            SymbolFlags::Generic
        );
        if (!optHeaderEndIndex.has_value())
        {
            return false;
        }

        const auto i = optHeaderEndIndex.value();

        return
            (parser.Peek(i) == TokenKind::PubKeyword) ||
            (parser.Peek(i) == TokenKind::StructKeyword);
    }

    static auto IsTypeBegin(const Parser& parser) -> bool
    {
        return
            IsTraitBegin(parser) ||
            IsStructBegin(parser);
    }

    static auto IsFunctionBegin(const Parser& parser) -> bool
    {
        return GetNamedSymbolHeaderEndIndex(
            parser,
            SymbolFlags::Generic | SymbolFlags::Paramized
        ).has_value();
    }

    static auto GetTokenKindSetUntil(
        const Parser& parser,
        const std::vector<TokenKind> predicates
    ) -> std::set<TokenKind>
    {
        std::set<TokenKind> tokenKindSet{};

        size_t i = 0;

        while (
            std::find(begin(predicates), end(predicates), parser.Peek(i).Kind) ==
            end(predicates)
            )
        {
            tokenKindSet.insert(parser.Peek(i).Kind);
            i++;
        }

        return tokenKindSet;
    }

    static auto IsInherentImplBegin(const Parser& parser) -> bool
    {
        if (parser.Peek(0) != TokenKind::ImplKeyword)
        {
            return false;
        }

        const auto tokenKindSet = GetTokenKindSetUntil(
            parser,
            {
                TokenKind::OpenBrace,
                TokenKind::CloseBrace,
                TokenKind::Semicolon,
            }
        );

        if (tokenKindSet.contains(TokenKind::ForKeyword))
        {
            return false;
        }

        return true;
    }

    static auto IsTraitImplBegin(const Parser& parser) -> bool
    {
        if (parser.Peek(0) != TokenKind::ImplKeyword)
        {
            return false;
        }

        const auto tokenKindSet = GetTokenKindSetUntil(
            parser,
            {
                TokenKind::OpenBrace,
                TokenKind::CloseBrace,
                TokenKind::Semicolon,
            }
        );

        if (!tokenKindSet.contains(TokenKind::ForKeyword))
        {
            return false;
        }

        return true;
    }

    static auto IsVarBegin(const Parser& parser) -> bool
    {
        const auto optHeaderEndIndex = GetNamedSymbolHeaderEndIndex(parser);
        if (!optHeaderEndIndex.has_value())
        {
            return false;
        }

        const auto i = optHeaderEndIndex.value();

        return
            (parser.Peek(i) != TokenKind::ModKeyword) &&
            (parser.Peek(i) != TokenKind::TraitKeyword) &&
            (parser.Peek(i) != TokenKind::PubKeyword) &&
            (parser.Peek(i) != TokenKind::StructKeyword);
    }

    static auto IsUseBegin(const Parser& parser) -> bool
    {
        return parser.Peek() == TokenKind::UseKeyword;
    }

    static auto IsSpecialSymbolNameSectionBegin(const Parser& parser) -> bool
    {
        return
            (parser.Peek(0) == TokenKind::SelfTypeKeyword) ||
            (parser.Peek(0) == TokenKind::IntKeyword) ||
            (parser.Peek(0) == TokenKind::Int8Keyword) ||
            (parser.Peek(0) == TokenKind::Int16Keyword) ||
            (parser.Peek(0) == TokenKind::Int32Keyword) ||
            (parser.Peek(0) == TokenKind::Int64Keyword) ||
            (parser.Peek(0) == TokenKind::UInt8Keyword) ||
            (parser.Peek(0) == TokenKind::UInt16Keyword) ||
            (parser.Peek(0) == TokenKind::UInt32Keyword) ||
            (parser.Peek(0) == TokenKind::UInt64Keyword) ||
            (parser.Peek(0) == TokenKind::Float32Keyword) ||
            (parser.Peek(0) == TokenKind::Float64Keyword) ||
            (parser.Peek(0) == TokenKind::BoolKeyword) ||
            (parser.Peek(0) == TokenKind::VoidKeyword);
    }

    static auto IsSymbolNameBegin(const Parser& parser) -> bool
    {
        return
            (parser.Peek() == TokenKind::ColonColon) ||
            (parser.Peek() == TokenKind::Ident) ||
            IsSpecialSymbolNameSectionBegin(parser);
    }

    static auto IsConstraintBegin(const Parser& parser) -> bool
    {
        return IsSymbolNameBegin(parser);
    }

    static auto RemoveConstrainedTypeParams(
        std::unordered_map<std::string_view, std::shared_ptr<const TypeParamSyntax>>& unconstrainedParamMap,
        const SymbolName& typeName
    ) -> void
    {
        std::for_each(begin(typeName.Sections), end(typeName.Sections),
        [&](const SymbolNameSection& section)
        {
            if (unconstrainedParamMap.contains(section.Name.String))
            {
                unconstrainedParamMap.erase(section.Name.String);
            }

            std::for_each(begin(section.TypeArgs), end(section.TypeArgs),
            [&](const SymbolName& arg)
            {
                RemoveConstrainedTypeParams(unconstrainedParamMap, arg);
            });
        });
    }

    static auto CreateUnconstrainedTypeParamMap(
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& params
    ) -> std::unordered_map<std::string_view, std::shared_ptr<const TypeParamSyntax>>
    {
        std::unordered_map<std::string_view, std::shared_ptr<const TypeParamSyntax>> unconstrainedParamMap{};
        std::for_each(begin(params), end(params),
        [&](const std::shared_ptr<const TypeParamSyntax>& param)
        {
            unconstrainedParamMap[param->GetName().String] = param;
        });

        return unconstrainedParamMap;
    }

    static auto DiagnoseUnconstrainedTypeParams(
        const std::unordered_map<std::string_view, std::shared_ptr<const TypeParamSyntax>>& unconstrainedParamMap
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::for_each(begin(unconstrainedParamMap), end(unconstrainedParamMap),
        [&](const auto& nameAndParamPair)
        {
            diagnostics.Add(CreateUnconstrainedTypeParamError(
                nameAndParamPair.second->GetName().SrcLocation
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto AreInherentImplTypeParamsConstrained(
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& params,
        const SymbolName& typeName
    ) -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto unconstrainedParamMap = CreateUnconstrainedTypeParamMap(params);

        RemoveConstrainedTypeParams(unconstrainedParamMap, typeName);

        diagnostics.Collect(
            DiagnoseUnconstrainedTypeParams(unconstrainedParamMap)
        );

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Void{ std::move(diagnostics) };
    }

    static auto AreTraitImplTypeParamsConstrained(
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& params,
        const SymbolName& traitName,
        const SymbolName& typeName
    ) -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto unconstrainedParamMap = CreateUnconstrainedTypeParamMap(params);

        RemoveConstrainedTypeParams(unconstrainedParamMap, traitName);
        RemoveConstrainedTypeParams(unconstrainedParamMap, typeName);

        diagnostics.Collect(
            DiagnoseUnconstrainedTypeParams(unconstrainedParamMap)
        );

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Void{ std::move(diagnostics) };
    }

    static auto ParseExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprSyntax>>;

    static auto ParseOptionalTypeArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>;

    static auto ParseStructConstructionExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructConstructionExprSyntax>>;

    static auto ParseStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IStmtSyntax>>;

    template<typename T>
    static auto ParseList(
        Parser& parser,
        const std::set<TokenKind>& terminators,
        const std::function<Expected<T>(const size_t)>& parseElement
    ) -> Expected<std::vector<T>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<TokenKind> terminatorsWithComma
        {
            begin(terminators),
            end  (terminators),
        };
        terminatorsWithComma.push_back(TokenKind::Comma);

        std::vector<T> elements{};
        bool isFirstElement = true;
        while (
            !parser.IsEnd() &&
            !terminators.contains(parser.Peek().Kind)
            )
        {
            if (isFirstElement)
            {
                isFirstElement = false;
            }
            else
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

                if (terminators.contains(parser.Peek().Kind))
                {
                    break;
                }
            }

            const auto optElement = diagnostics.Collect(
                parseElement(elements.size())
            );
            if (optElement.has_value())
            {
                elements.push_back(optElement.value());
                continue;
            }

            parser.DiscardUntil(DiscardKind::Exclusive, terminatorsWithComma);
        }

        return Expected<std::vector<T>>
        {
            std::move(elements),
            std::move(diagnostics),
        };
    }

    static auto ParseName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<Ident>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::Ident)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Ident
            ));
            return std::move(diagnostics);
        }

        const auto& nameToken = parser.Eat();

        return Expected
        {
            Ident
            {
                nameToken.SrcLocation,
                nameToken.String,
            },
            std::move(diagnostics),
        };
    }

    static auto ParseNestedName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<Ident>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<Ident> nestedName{};

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return std::move(diagnostics);
        }

        nestedName.push_back(std::move(optName.value()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto optName = diagnostics.Collect(ParseName(parser, scope));
            if (!optName.has_value())
            {
                return std::move(diagnostics);
            }

            nestedName.push_back(std::move(optName.value()));
        }

        return Expected{ std::move(nestedName), std::move(diagnostics) };
    }

    static auto ParseSpecialSymbolNameSection(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolNameSection>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!IsSpecialSymbolNameSectionBegin(parser))
        {
            diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
            return std::move(diagnostics);
        }

        const auto& token = parser.Eat();

        std::string name{ TokenKindToKeywordMap.at(token.Kind) };
        
        return Expected
        {
            SymbolNameSection{ Ident{ token.SrcLocation, std::move(name) } },
            std::move(diagnostics),
        };
    }

    static auto ParseSymbolNameSection(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolNameSection>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (IsSpecialSymbolNameSectionBegin(parser))
        {
            const auto optSection = diagnostics.Collect(
                ParseSpecialSymbolNameSection(parser, scope)
            );
            if (!optSection.has_value())
            {
                return std::move(diagnostics);
            }

            return Expected{ optSection.value(), std::move(diagnostics) };
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optTypeArgs = diagnostics.Collect(
            ParseOptionalTypeArgs(parser, scope)
        );
        if (!optTypeArgs.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            SymbolNameSection
            {
                optName.value(),
                optTypeArgs.value(),
            },
            std::move(diagnostics),
        };
    }

    static auto ParseSymbolName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<SymbolName>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (parser.Peek() == TokenKind::ColonColon)
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            parser.Eat();
        }
        
        std::vector<SymbolNameSection> sections{};

        const auto optSection = diagnostics.Collect(
            ParseSymbolNameSection(parser, scope)
        );
        if (!optSection.has_value())
        {
            return std::move(diagnostics);
        }

        sections.push_back(std::move(optSection.value()));

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto optSection = diagnostics.Collect(
                ParseSymbolNameSection(parser, scope)
            );
            if (!optSection.has_value())
            {
                return std::move(diagnostics);
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
            std::move(diagnostics),
        };
    }

    static auto ParseTypeName(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const RefParsingKind refParsingKind
    ) -> Expected<TypeName>
    {
        auto diagnostics = DiagnosticBag::Create();

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
                modifiers.push_back(TypeNameModifier::AutoStrongPtr);
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

        const auto optSymbolName = diagnostics.Collect(
            ParseSymbolName(parser, scope)
        );
        if (!optSymbolName.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            TypeName
            {
                optSymbolName.value(),
                modifiers,
            },
            std::move(diagnostics),
        };
    }

    static auto ParseTypeParamNames(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<Ident>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optNames = diagnostics.Collect(ParseList<Ident>(
            parser,
            { TokenKind::CloseBracket },
            [&](const size_t index) { return ParseName(parser, scope); }
        ));
        if (!optNames.has_value())
        {
            return std::move(diagnostics);
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

        if (optNames.value().empty())
        {
            const SrcLocation paramsSrcLocation
            {
                beginSrcLocation,
                parser.GetLastSrcLocation(),
            };
            diagnostics.Add(CreateEmptyTypeParamsError(paramsSrcLocation));
            return std::move(diagnostics);
        }

        return Expected{ optNames.value(), std::move(diagnostics) };
    }

    static auto ParseTypeParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& parentParams = {}
    ) -> Expected<std::vector<std::shared_ptr<const TypeParamSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const TypeParamSyntax>> params{};
        
        std::transform(
            begin(parentParams),
            end  (parentParams),
            back_inserter(params),
            [&](const std::shared_ptr<const TypeParamSyntax>& param)
            {
                return CloneTypeParamInScope(param, scope);
            }
        );

        const auto optNames = diagnostics.Collect(
            ParseTypeParamNames(parser, scope)
        );
        if (!optNames.has_value())
        {
            return std::move(diagnostics);
        }

        std::transform(
            begin(optNames.value()),
            end  (optNames.value()),
            back_inserter(params),
            [&](const Ident& name)
            {
                return std::make_shared<const TypeParamSyntax>(
                    name.SrcLocation,
                    scope,
                    name,
                    params.size()
                );
            }
        );

        return Expected{ std::move(params), std::move(diagnostics) };
    }

    static auto ParseOptionalTypeParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& parentParams = {}
    ) -> Expected<std::vector<std::shared_ptr<const TypeParamSyntax>>>
    {
        if (parser.Peek() != TokenKind::OpenBracket)
        {
            std::vector<std::shared_ptr<const TypeParamSyntax>> params{};
            std::transform(
                begin(parentParams),
                end  (parentParams),
                back_inserter(params),
                [&](const std::shared_ptr<const TypeParamSyntax>& param)
                {
                    return CloneTypeParamInScope(param, scope);
                }
            );

            return Expected{ std::move(params), DiagnosticBag::Create() };
        }

        return ParseTypeParams(parser, scope, parentParams);
    }

    static auto ParseTypeArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optArgs = diagnostics.Collect(ParseList<SymbolName>(
            parser,
            { TokenKind::CloseBracket },
            [&](const size_t index) { return ParseSymbolName(parser, scope); }
        ));
        if (!optArgs.has_value())
        {
            return std::move(diagnostics);

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

        if (optArgs.value().empty())
        {
            const SrcLocation argsSrcLocation
            {
                beginSrcLocation,
                parser.GetLastSrcLocation(),
            };
            diagnostics.Add(CreateEmptyTypeArgsError(argsSrcLocation));
            return std::move(diagnostics);
        }

        return Expected{ optArgs.value(), std::move(diagnostics) };
    }

    static auto ParseOptionalTypeArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            return Expected
            {
                std::vector<SymbolName>{},
                std::move(diagnostics),
            };
        }

        const auto optArgs = diagnostics.Collect(ParseTypeArgs(parser, scope));
        if (!optArgs.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ std::move(optArgs.value()), std::move(diagnostics) };
    }

    static auto ParseAttribute(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AttributeSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optStructConstructionExpr = diagnostics.Collect(
            ParseStructConstructionExpr(parser, scope)
        );
        if (!optStructConstructionExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const AttributeSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optStructConstructionExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseAttributes(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const AttributeSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const AttributeSyntax>> attributes{};
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

        return Expected{ std::move(attributes), std::move(diagnostics) };
    }

    static auto ParseParam(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const size_t index
    ) -> Expected<std::shared_ptr<const NormalParamVarSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optAttributes = diagnostics.Collect(ParseAttributes(
            parser,
            scope
        ));
        if (!optAttributes.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Allow)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const NormalParamVarSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optName.value(),
                optTypeName.value(),
                optAttributes.value(),
                index
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseParams(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const NormalParamVarSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optParams = diagnostics.Collect(
            ParseList<std::shared_ptr<const NormalParamVarSyntax>>(
                parser,
                { TokenKind::CloseParen },
                [&](const size_t index)
                {
                    return ParseParam(parser, scope, index);
                }
            )
        );
        if (!optParams.has_value())
        {
            return std::move(diagnostics);
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

        return Expected{ optParams.value(), std::move(diagnostics) };
    }


    static auto ParseModifiers(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        std::vector<Modifier> allowedModifiers
    ) -> Diagnosed<std::map<Modifier, Token>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::map<Modifier, Token> modifierToTokenMap{};
        while (
            !parser.IsEnd() &&
            (parser.Peek() != TokenKind::ColonColon) &&
            (parser.Peek() != TokenKind::OpenBracket) &&
            (parser.Peek() != TokenKind::OpenParen) &&
            (parser.Peek() != TokenKind::Ident)
            )
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

        if (parser.Peek() == TokenKind::ColonColon)
        {
            if (modifierToTokenMap.empty())
            {
                diagnostics.Add(CreateEmptyModifiersError(parser.Peek()));
            }
            else
            {
                parser.Eat();
            }
        }

        return Diagnosed
        {
            std::move(modifierToTokenMap),
            std::move(diagnostics),
        };
    }

    static auto ParseNamedSymbolHeader(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        std::vector<Modifier> allowedModifiers,
        const SymbolFlags flags,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& parentTypeParams = {}
    ) -> Expected<NamedSymbolHeader>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        auto modifiers = diagnostics.Collect(
            ParseModifiers(parser, scope, allowedModifiers)
        );

        auto optAttributes = diagnostics.Collect(
            ParseAttributes(parser, scope)
        );
        if (!optAttributes.has_value())
        {
            return std::move(diagnostics);
        }

        Ident* name = nullptr;
        std::vector<Ident> nestedName{};
        std::shared_ptr<Scope> bodyScope{};
        if (flags & SymbolFlags::NestedName)
        {
            ACE_ASSERT(flags & SymbolFlags::NamedBodyScope);

            const auto optNestedName = diagnostics.Collect(
                ParseNestedName(parser, scope)
            );
            if (!optNestedName.has_value())
            {
                return std::move(diagnostics);
            }

            nestedName = optNestedName.value();
            name = &nestedName.back();

            std::vector<std::shared_ptr<Scope>> scopes{};
            scopes.push_back(scope);
            std::transform(
                begin(nestedName),
                end  (nestedName),
                back_inserter(scopes),
                [&](const Ident& name)
                {
                    return scopes.back()->GetOrCreateChild(name.String);
                }
            );

            bodyScope = scopes.back();
        }
        else
        {
            const auto optName = diagnostics.Collect(ParseName(parser, scope));
            if (!optName.has_value())
            {
                return std::move(diagnostics);
            }

            nestedName.push_back(optName.value());
            name = &nestedName.back();

            bodyScope = (flags & SymbolFlags::NamedBodyScope) ? 
                scope->GetOrCreateChild(name->String) :
                scope->GetOrCreateChild(AnonymousIdent::Create(name->String));
        }

        std::vector<std::shared_ptr<const TypeParamSyntax>> typeParams{};
        if (flags & SymbolFlags::Generic)
        {
            const auto optTypeParams = diagnostics.Collect(
                ParseOptionalTypeParams(parser, bodyScope, parentTypeParams)
            );
            if (!optTypeParams.has_value())
            {
                return std::move(diagnostics);
            }

            typeParams = std::move(optTypeParams.value());
        }

        std::vector<std::shared_ptr<const NormalParamVarSyntax>> params{};
        if (flags & SymbolFlags::Paramized)
        {
            const auto optParams = diagnostics.Collect(
                ParseParams(parser, bodyScope)
            );
            if (!optParams.has_value())
            {
                return std::move(diagnostics);
            }

            params = std::move(optParams.value());
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            NamedSymbolHeader
            {
                std::move(modifiers),
                std::move(optAttributes.value()),
                *name,
                std::move(nestedName),
                std::move(bodyScope),
                std::move(typeParams),
                std::move(params),
            },
            std::move(diagnostics),
        };
    }

    static auto ParseTraitNames(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<SymbolName>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<SymbolName> traitNames{};
        bool isFirstTraitName = true;
        while (
            !parser.IsEnd() &&
            !(
                parser.Peek() == TokenKind::Comma ||
                parser.Peek() == TokenKind::Semicolon ||
                parser.Peek() == TokenKind::OpenBrace
            )
            )
        {
            if (isFirstTraitName)
            {
                isFirstTraitName = false;
            }
            else
            {
                if (parser.Peek() == TokenKind::Plus)
                {
                    parser.Eat();
                }
                else
                {
                    diagnostics.Add(CreateMissingTokenError(
                        parser.GetLastSrcLocation(),
                        TokenKind::Plus
                    ));
                }
            }

            const auto optTraitName = diagnostics.Collect(
                ParseSymbolName(parser, scope)
            );
            if (optTraitName.has_value())
            {
                traitNames.push_back(optTraitName.value());
                continue;
            }

            parser.DiscardUntil(
                DiscardKind::Exclusive,
                { 
                    TokenKind::Plus,
                    TokenKind::Comma,
                    TokenKind::Semicolon,
                    TokenKind::OpenBrace
                }
            );
        }

        if (traitNames.empty())
        {
            const auto srcLocation = SrcLocation::CreateInterstice(
                parser.GetLastSrcLocation(),
                parser.Peek().SrcLocation
            );
            diagnostics.Add(CreateExpectedTraitError(srcLocation));
        }

        return Expected{ std::move(traitNames), std::move(diagnostics) };
    }

    static auto ParseConstraint(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ConstraintSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optTypeName = diagnostics.Collect(
            ParseSymbolName(parser, scope)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Colon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Colon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTraitNames = diagnostics.Collect(
            ParseTraitNames(parser, scope)
        );
        if (!optTraitNames.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const ConstraintSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optTypeName.value(),
                optTraitNames.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseConstraints(
        Parser& parser,
        const NamedSymbolHeader& header
    ) -> Expected<std::vector<std::shared_ptr<const ConstraintSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::WhereKeyword)
        {
            return Expected
            {
                std::vector<std::shared_ptr<const ConstraintSyntax>>{},
                std::move(diagnostics),
            };
        }

        const auto whereToken = parser.Eat();

        const auto optConstraints = diagnostics.Collect(
            ParseList<std::shared_ptr<const ConstraintSyntax>>(
                parser,
                { TokenKind::Semicolon, TokenKind::OpenBrace },
                [&](const size_t index)
                {
                    return ParseConstraint(parser, header.BodyScope);
                }
            )
        );
        if (!optConstraints.has_value())
        {
            return std::move(diagnostics);
        }

        if (optConstraints.value().empty())
        {
            const auto constraintsSrcLocation = SrcLocation::CreateInterstice(
                whereToken.SrcLocation,
                parser.Peek().SrcLocation
            );
            diagnostics.Add(CreateEmptyConstraintsError(
                constraintsSrcLocation
            ));
        }

        if (!optConstraints.value().empty() && header.TypeParams.empty())
        {
            diagnostics.Add(CreateConstrainedNonGenericSymbolError(SrcLocation{
                optConstraints.value().front()->GetSrcLocation(),
                optConstraints.value().back ()->GetSrcLocation(),
            }));
            return Expected
            {
                std::vector<std::shared_ptr<const ConstraintSyntax>>{},
                std::move(diagnostics),
            };
        }

        return Expected{ optConstraints.value(), std::move(diagnostics) };
    }

    static auto ParseArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<std::shared_ptr<const IExprSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optArgs = diagnostics.Collect(
            ParseList<std::shared_ptr<const IExprSyntax>>(
                parser,
                { TokenKind::CloseParen },
                [&](const size_t index) { return ParseExpr(parser, scope); }
            )
        );
        if (!optArgs.has_value())
        {
            return std::move(diagnostics);
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

        return Expected{ optArgs.value(), std::move(diagnostics) };
    }

    static auto ParseLiteralExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const LiteralExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& literalToken = parser.Eat();

        const auto optLiteralKind = diagnostics.Collect(
            GetLiteralKind(literalToken)
        );
        if (!optLiteralKind.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const LiteralExprSyntax>(
                literalToken.SrcLocation,
                scope,
                optLiteralKind.value(),
                literalToken.String
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseSelfSymbolLiteralExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SymbolLiteralExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::SelfKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::SelfKeyword
            ));
            return std::move(diagnostics);
        }

        const auto& token = parser.Eat();

        const SymbolName name
        {
            SymbolNameSection{ Ident{ token.SrcLocation, SpecialIdent::Self } },
            SymbolNameResolutionScope::Local,
        };

        return Expected
        {
            std::make_shared<const SymbolLiteralExprSyntax>(
                token.SrcLocation,
                scope,
                name
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseSymbolLiteralExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SymbolLiteralExprSyntax>>
    {
        if (parser.Peek() == TokenKind::SelfKeyword)
        {
            return ParseSelfSymbolLiteralExpr(parser, scope);
        }

        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optName = diagnostics.Collect(ParseSymbolName(
            parser,
            scope
        ));
        if (!optName.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const SymbolLiteralExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optName.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseStructConstructionExprArgs(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::vector<StructConstructionExprArg>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optArgs = diagnostics.Collect(
            ParseList<StructConstructionExprArg>(
                parser,
                { TokenKind::CloseBrace },
                [&](const size_t index) -> Expected<StructConstructionExprArg>
                {
                    auto diagnostics = DiagnosticBag::Create();

                    const auto optName = diagnostics.Collect(
                        ParseName(parser, scope)
                    );
                    if (!optName.has_value())
                    {
                        return std::move(diagnostics);
                    }

                    std::optional<std::shared_ptr<const IExprSyntax>> optValue{};
                    if (parser.Peek() == TokenKind::Colon)
                    {
                        parser.Eat();

                        optValue = diagnostics.Collect(
                            ParseExpr(parser, scope)
                        );
                        if (!optValue.has_value())
                        {
                            return std::move(diagnostics);
                        }
                    }
                    
                    return Expected
                    {
                        StructConstructionExprArg{ optName.value(), optValue },
                        std::move(diagnostics),
                    };
                }
            )
        );
        if (!optArgs.has_value())
        {
            return std::move(diagnostics);
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

        return Expected{ optArgs.value(), std::move(diagnostics) };
    }

    static auto ParseStructConstructionExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructConstructionExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != SpecialIdent::New)
        {
            diagnostics.Add(CreateUnexpectedTokenExpectedNewError(
                parser.Peek()
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(ParseSymbolName(
            parser,
            scope
        ));
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        auto optArgs = diagnostics.Collect(ParseStructConstructionExprArgs(
            parser,
            scope
        ));
        if (!optArgs.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const StructConstructionExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optTypeName.value(),
                std::move(optArgs.value())
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseCastExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const CastExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::CastKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CastKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Allow)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const CastExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optTypeName.value(),
                optExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseAddressOfExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AddressOfExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::AddressOfKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::AddressOfKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const AddressOfExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseSizeOfExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SizeOfExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::SizeOfKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::SizeOfKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Allow)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const SizeOfExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optTypeName.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseDerefAsExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const DerefAsExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::DerefAsKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::DerefAsKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Allow)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const DerefAsExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optTypeName.value(),
                optExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseTypeInfoPtrExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TypeInfoPtrExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::TypeInfoPtrKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::TypeInfoPtrKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(ParseTypeName(
            parser,
            scope,
            RefParsingKind::Allow
        ));
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const TypeInfoPtrExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optTypeName.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseVtblPtrExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const VtblPtrExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::VtblPtrKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::VtblPtrKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Allow)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Comma)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Comma
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTraitName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Allow)
        );
        if (!optTraitName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const VtblPtrExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optTypeName.value(),
                optTraitName.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseCopyStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const CopyStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::CopyKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CopyKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optSrcExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optSrcExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Comma)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Comma
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optDstExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optDstExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

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

        return Expected
        {
            std::make_shared<const CopyStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optTypeName.value(),
                optSrcExpr.value(),
                optDstExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseDropStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const DropStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::DropKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::DropKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseBracket)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseBracket
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

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

        return Expected
        {
            std::make_shared<const DropStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optTypeName.value(),
                optExpr.value()
            ),
            std::move(diagnostics),
        };
    }


    static auto ParseExprExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExprExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::OpenParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::CloseParen)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::CloseParen
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExprExprSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseKeywordExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprSyntax>>
    {
        switch (parser.Peek().Kind)
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

            case TokenKind::TypeInfoPtrKeyword:
            {
                return ParseTypeInfoPtrExpr(parser, scope);
            }

            case TokenKind::VtblPtrKeyword:
            {
                return ParseVtblPtrExpr(parser, scope);
            }

            default:
            {
                auto diagnostics = DiagnosticBag::Create();
                diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
                return std::move(diagnostics);
            }
        }
    }

    static auto ParsePrimaryExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprSyntax>>
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

        auto diagnostics = DiagnosticBag::Create();
        diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
        return std::move(diagnostics);
    }

    static auto ParseSecondaryExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optPrimaryExpr = diagnostics.Collect(
            ParsePrimaryExpr(parser, scope)
        );
        if (!optPrimaryExpr.has_value())
        {
            return std::move(diagnostics);
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
                    return std::move(diagnostics);
                }

                expr = std::make_shared<const MemberAccessExprSyntax>(
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
                    return std::move(diagnostics);
                }

                expr = std::make_shared<const CallExprSyntax>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    expr,
                    optArgs.value()
                );
            }
        }

        return Expected
        {
            expr,
            std::move(diagnostics),
        };
    }

    static auto ParseUnaryExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<LocatedOp> ops{};
        while (IsPrefixOp(parser.Peek().Kind))
        {
            const auto& opToken = parser.Eat();
            ops.emplace_back(opToken.SrcLocation, opToken.Kind);
        }

        const auto optSecondaryExpr = diagnostics.Collect(
            ParseSecondaryExpr(parser, scope)
        );
        if (!optSecondaryExpr.has_value())
        {
            return std::move(diagnostics);
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
            std::move(diagnostics),
        };
    }

    static auto ParseExpr(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IExprSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optUnaryExpr = diagnostics.Collect(
            ParseUnaryExpr(parser, scope)
        );
        if (!optUnaryExpr.has_value())
        {
            return std::move(diagnostics);
        }

        std::vector<std::shared_ptr<const IExprSyntax>> exprs{};
        std::vector<LocatedOp> ops{};

        exprs.push_back(optUnaryExpr.value());
        
        while (IsBinaryOp(parser.Peek().Kind))
        {
            const auto& opToken = parser.Eat();
            ops.emplace_back(opToken.SrcLocation, opToken.Kind);

            const auto optUnaryExpr = diagnostics.Collect(
                ParseUnaryExpr(parser, scope)
            );
            if (!optUnaryExpr.has_value())
            {
                return std::move(diagnostics);
            }

            exprs.push_back(optUnaryExpr.value());
        }

        if (ops.empty())
        {
            ACE_ASSERT(exprs.size() == 1);

            return Expected
            {
                exprs.front(),
                std::move(diagnostics),
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
                    const auto precedence = GetBinaryOpPrecedence(op.TokenKind);

                    if (precedence == precedenceLevel)
                    {
                        didCollapseAny = true;

                        const auto lhsExpr = exprs.at(i);
                        const auto rhsExpr = exprs.at(i + 1);

                        ops.erase(begin(ops) + i);
                        exprs.erase(begin(exprs) + i + 1);

                        const SrcLocation srcLocation
                        {
                            lhsExpr->GetSrcLocation(),
                            rhsExpr->GetSrcLocation(),
                        };
                        exprs.at(i) = CreateCollapsedBinaryExpr(
                            srcLocation,
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
            std::move(diagnostics),
        };
    }

    static auto ParseBlockStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const BlockStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto bodyScope = scope->CreateChild();

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        std::vector<std::shared_ptr<const IStmtSyntax>> stmts{};
        while (!parser.IsEnd() && (parser.Peek() != TokenKind::CloseBrace))
        {
            const auto optStmt = diagnostics.Collect(
                ParseStmt(parser, bodyScope)
            );
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
            std::make_shared<const BlockStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                bodyScope,
                stmts
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseExprStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExprStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const ExprStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                optExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseAssignmentStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const SimpleAssignmentStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optLHSExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optLHSExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Equals)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Equals
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optRHSExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optRHSExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const SimpleAssignmentStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optLHSExpr.value(),
                optRHSExpr.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseCompoundAssignmentStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const CompoundAssignmentStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optLhsExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optLhsExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (!IsCompoundAssignmentOp(parser.Peek().Kind))
        {
            diagnostics.Add(CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
                parser.Peek()
            ));
            return std::move(diagnostics);
        }

        const auto opToken = parser.Eat();

        const auto optRhsExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optRhsExpr.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const CompoundAssignmentStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optLhsExpr.value(),
                optRhsExpr.value(),
                opToken.SrcLocation,
                GetBinaryOp(opToken.Kind)
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseVarStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const VarStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            {},
            SymbolFlags::None
        ));

        const auto& header = optHeader.value();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Allow)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        std::optional<std::shared_ptr<const IExprSyntax>> optAssignedExpr{};
        if (parser.Peek() == TokenKind::Equals)
        {
            parser.Eat();

            optAssignedExpr = diagnostics.Collect(ParseExpr(parser, scope));
            if (!optAssignedExpr.has_value())
            {
                return std::move(diagnostics);
            }
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const VarStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                header.Name,
                optTypeName.value(),
                header.Attributes,
                optAssignedExpr
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseIfBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprSyntax>, std::shared_ptr<const BlockStmtSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::IfKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::IfKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optBlock = diagnostics.Collect(
            ParseBlockStmt(parser, scope)
        );
        if (!optBlock.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::pair{ optCondition.value(), optBlock.value() },
            std::move(diagnostics),
        };
    }

    static auto ParseElifBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::pair<std::shared_ptr<const IExprSyntax>, std::shared_ptr<const BlockStmtSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::ElifKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ElifKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optBlock = diagnostics.Collect(
            ParseBlockStmt(parser, scope)
        );
        if (!optBlock.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::pair{ optCondition.value(), optBlock.value() },
            std::move(diagnostics),
        };
    }

    static auto ParseElseBlock(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const BlockStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::ElseKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ElseKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optBlock = diagnostics.Collect(
            ParseBlockStmt(parser, scope)
        );
        if (!optBlock.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ optBlock.value(), std::move(diagnostics) };
    }

    static auto ParseIfStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IfStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        std::vector<std::shared_ptr<const IExprSyntax>> conditions{};
        std::vector<std::shared_ptr<const BlockStmtSyntax>> bodies{};

        const auto optIfBlock = diagnostics.Collect(
            ParseIfBlock(parser, scope)
        );
        if (!optIfBlock.has_value())
        {
            return std::move(diagnostics);
        }

        conditions.push_back(optIfBlock.value().first);
        bodies.push_back(optIfBlock.value().second);

        while (parser.Peek() == TokenKind::ElifKeyword)
        {
            const auto optElifBlock = diagnostics.Collect(
                ParseElifBlock(parser, scope)
            );
            if (!optElifBlock.has_value())
            {
                return std::move(diagnostics);
            }

            conditions.push_back(optElifBlock.value().first);
            bodies.push_back(optElifBlock.value().second);
        }

        if (parser.Peek() == TokenKind::ElseKeyword)
        {
            const auto optElseBlock = diagnostics.Collect(
                ParseElseBlock(parser, scope)
            );
            if (!optElseBlock.has_value())
            {
                return std::move(diagnostics);
            }

            bodies.push_back(optElseBlock.value());
        }

        return Expected
        {
            std::make_shared<const IfStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                conditions,
                bodies
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseWhileStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const WhileStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::WhileKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::WhileKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optBlock = diagnostics.Collect(
            ParseBlockStmt(parser, scope)
        );
        if (!optBlock.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const WhileStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optCondition.value(),
                optBlock.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseRetStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const RetStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::RetKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::RetKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        std::optional<std::shared_ptr<const IExprSyntax>> optExpr{};
        if (parser.Peek() != TokenKind::Semicolon)
        {
            optExpr = diagnostics.Collect(ParseExpr(parser, scope));
            if (!optExpr.has_value())
            {
                return std::move(diagnostics);
            }
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

        return Expected
        {
            std::make_shared<const RetStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optExpr
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseExitStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ExitStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::ExitKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ExitKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() == TokenKind::Semicolon)
        {
            parser.Eat();
        }
        else
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
        }

        return Expected
        {
            std::make_shared<const ExitStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseAssertStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const AssertStmtSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        if (parser.Peek() != TokenKind::AssertKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::AssertKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optCondition = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optCondition.has_value())
        {
            return std::move(diagnostics);
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

        return Expected
        {
            std::make_shared<const AssertStmtSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                optCondition.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseKeywordStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IStmtSyntax>>
    {
        switch (parser.Peek().Kind)
        {
            case TokenKind::IfKeyword:
            {
                return ParseIfStmt(parser, scope);
            }

            case TokenKind::WhileKeyword:
            {
                return ParseWhileStmt(parser, scope);
            }

            case TokenKind::RetKeyword:
            {
                return ParseRetStmt(parser, scope);
            }

            case TokenKind::ExitKeyword:
            {
                return ParseExitStmt(parser, scope);
            }

            case TokenKind::AssertKeyword:
            {
                return ParseAssertStmt(parser, scope);
            }

            case TokenKind::CopyKeyword:
            {
                return ParseCopyStmt(parser, scope);
            }

            case TokenKind::DropKeyword:
            {
                return ParseDropStmt(parser, scope);
            }

            default:
            {
                auto diagnostics = DiagnosticBag::Create();
                diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
                return std::move(diagnostics);
            }
        }
    }

    static auto ParseStmt(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const IStmtSyntax>>
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

        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optExpr.has_value())
        {
            return std::move(diagnostics);
        }

        const bool isSemicolon = parser.Peek() == TokenKind::Semicolon;
        const bool isAssignment = parser.Peek() == TokenKind::Equals;
        const bool isCompoundAssignment =
            IsCompoundAssignmentOp(parser.Peek().Kind);

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
                std::make_shared<const ExprStmtSyntax>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    optExpr.value()
                ),
                std::move(diagnostics),
            };
        }

        const auto opToken = parser.Eat();

        const auto optRhsExpr = diagnostics.Collect(ParseExpr(parser, scope));
        if (!optRhsExpr.has_value())
        {
            return std::move(diagnostics);
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
                std::make_shared<const SimpleAssignmentStmtSyntax>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    scope,
                    optExpr.value(),
                    optRhsExpr.value()
                ),
                std::move(diagnostics),
            };
        }

        if (isCompoundAssignment)
        {
            return Expected
            {
                std::make_shared<const CompoundAssignmentStmtSyntax>(
                    SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                    scope,
                    optExpr.value(),
                    optRhsExpr.value(),
                    opToken.SrcLocation,
                    GetBinaryOp(opToken.Kind)
                ),
                std::move(diagnostics),
            };
        }

        ACE_UNREACHABLE();
    }

    static auto ParseInherentImplFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& parentTypeParams
    ) -> Expected<std::shared_ptr<const FunctionSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::Pub, Modifier::StrongPtr, Modifier::Self },
            SymbolFlags::Generic | SymbolFlags::Paramized,
            parentTypeParams
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        auto accessModifier = AccessModifier::Priv;
        if (header.ModifierToTokenMap.contains(Modifier::Pub))
        {
            accessModifier = AccessModifier::Pub;
        }

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optConstraints = diagnostics.Collect(
            ParseConstraints(parser, header)
        );
        if (!optConstraints.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optBlock = diagnostics.Collect(
            ParseBlockStmt(parser, header.BodyScope)
        );
        if (!optBlock.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optSelfParam = diagnostics.Collect(CreateSelfParam(header));

        return Expected
        {
            std::make_shared<const FunctionSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetSrcLocation() },
                header.BodyScope,
                accessModifier,
                header.Name,
                optTypeName.value(),
                header.Attributes,
                CreateImplSelf(header.BodyScope, selfTypeName),
                optSelfParam,
                header.Params,
                optBlock,
                header.TypeParams,
                optConstraints.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseInherentImpl(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const InherentImplSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto bodyScope = scope->CreateChild();

        if (parser.Peek() != TokenKind::ImplKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ImplKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeParams = diagnostics.Collect(
            ParseOptionalTypeParams(parser, bodyScope)
        );
        if (!optTypeParams.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optTypeName = diagnostics.Collect(
            ParseSymbolName(parser, scope)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        const bool areTypeParamsConstrained = diagnostics.Collect(
            AreInherentImplTypeParamsConstrained(
                optTypeParams.value(),
                optTypeName.value()
            )
        );
        if (!areTypeParamsConstrained)
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        std::vector<std::shared_ptr<const FunctionSyntax>> functions{};
        while (!parser.IsEnd() && (parser.Peek() != TokenKind::CloseBrace))
        {
            if (IsFunctionBegin(parser))
            {
                const auto optFunction = diagnostics.Collect(ParseInherentImplFunction(
                    parser,
                    bodyScope,
                    optTypeName.value(),
                    optTypeParams.value()
                ));
                if (optFunction.has_value())
                {
                    functions.push_back(optFunction.value());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
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
            std::make_shared<const InherentImplSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                bodyScope,
                optTypeParams.value(),
                optTypeName.value(),
                std::vector<std::shared_ptr<const ConstraintSyntax>>{},
                functions
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseTraitImplFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& selfTypeName,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& parentTypeParams
    ) -> Expected<std::shared_ptr<const FunctionSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::StrongPtr, Modifier::Self },
            SymbolFlags::Generic | SymbolFlags::Paramized,
            parentTypeParams
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }
        
        const auto optConstraints = diagnostics.Collect(
            ParseConstraints(parser, header)
        );
        if (!optConstraints.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optBlock = diagnostics.Collect(
            ParseBlockStmt(parser, header.BodyScope)
        );
        if (!optBlock.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optSelfParam = diagnostics.Collect(
            CreateSelfParam(header)
        );

        return Expected
        {
            std::make_shared<const FunctionSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetSrcLocation() },
                header.BodyScope,
                AccessModifier::Pub,
                header.Name,
                optTypeName.value(),
                header.Attributes,
                CreateImplSelf(header.BodyScope, selfTypeName),
                optSelfParam,
                header.Params,
                optBlock,
                header.TypeParams,
                optConstraints.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseTraitImpl(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TraitImplSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();
        const auto bodyScope = scope->CreateChild();

        if (parser.Peek() != TokenKind::ImplKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ImplKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeParams = diagnostics.Collect(
            ParseOptionalTypeParams(parser, bodyScope)
        );
        if (!optTypeParams.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optTraitName = diagnostics.Collect(
            ParseSymbolName(parser, scope)
        );
        if (!optTraitName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::ForKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ForKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optTypeName = diagnostics.Collect(
            ParseSymbolName(parser, scope)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        const bool areTypeParamsConstrained = diagnostics.Collect(
            AreTraitImplTypeParamsConstrained(
                optTypeParams.value(),
                optTraitName.value(),
                optTypeName.value()
            )
        );
        if (!areTypeParamsConstrained)
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        std::vector<std::shared_ptr<const FunctionSyntax>> functions{};
        while (!parser.IsEnd() && (parser.Peek() != TokenKind::CloseBrace))
        {
            if (IsFunctionBegin(parser))
            {
                const auto optFunction = diagnostics.Collect(ParseTraitImplFunction(
                    parser,
                    bodyScope,
                    optTypeName.value(),
                    optTypeParams.value()
                ));
                if (optFunction.has_value())
                {
                    functions.push_back(optFunction.value());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
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
            std::make_shared<const TraitImplSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                bodyScope,
                optTypeParams.value(),
                optTraitName.value(),
                optTypeName.value(),
                std::vector<std::shared_ptr<const ConstraintSyntax>>{},
                CreateImplSelf(bodyScope, optTypeName.value()),
                functions
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseFunction(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const FunctionSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::Pub, Modifier::Extern },
            SymbolFlags::Generic | SymbolFlags::Paramized
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        auto accessModifier = AccessModifier::Priv;
        if (header.ModifierToTokenMap.contains(Modifier::Pub))
        {
            accessModifier = AccessModifier::Pub;
        }

        const bool isExtern =
            header.ModifierToTokenMap.contains(Modifier::Extern);

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, header.BodyScope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optConstraints = diagnostics.Collect(
            ParseConstraints(parser, header)
        );
        if (!optConstraints.has_value())
        {
            return std::move(diagnostics);
        }

        std::optional<std::shared_ptr<const BlockStmtSyntax>> optBlock{};
        if (isExtern)
        {
            if (parser.Peek() != TokenKind::Semicolon)
            {
                diagnostics.Add(CreateUnexpectedTokenError(
                    parser.Peek(),
                    TokenKind::Semicolon
                ));
                return std::move(diagnostics);
            }

            parser.Eat();
        }
        else
        {
            optBlock = diagnostics.Collect(
                ParseBlockStmt(parser, header.BodyScope)
            );
            if (!optBlock.has_value())
            {
                return std::move(diagnostics);
            }
        }

        return Expected
        {
            std::make_shared<const FunctionSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                header.BodyScope,
                accessModifier,
                header.Name,
                optTypeName.value(),
                header.Attributes,
                std::nullopt,
                std::nullopt,
                header.Params,
                optBlock,
                header.TypeParams,
                optConstraints.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseGlobalVar(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const GlobalVarSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::Pub },
            SymbolFlags::None
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        auto accessModifier = AccessModifier::Priv;
        if (header.ModifierToTokenMap.contains(Modifier::Pub))
        {
            accessModifier = AccessModifier::Pub;
        }

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        if (parser.Peek() != TokenKind::Semicolon)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::Semicolon
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        return Expected
        {
            std::make_shared<const GlobalVarSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                header.Name,
                optTypeName.value(),
                header.Attributes,
                accessModifier
            ),
            std::move(diagnostics),
        };
    }

    static auto ParsePrototype(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& parentTraitName,
        const std::vector<std::shared_ptr<const TypeParamSyntax>>& parentTypeParams,
        const size_t index
    ) -> Expected<std::shared_ptr<const PrototypeSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::StrongPtr, Modifier::Self },
            SymbolFlags::Generic | SymbolFlags::Paramized,
            parentTypeParams
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optConstraints = diagnostics.Collect(
            ParseConstraints(parser, header)
        );
        if (!optConstraints.has_value())
        {
            return std::move(diagnostics);
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

        const auto optSelfParam = diagnostics.Collect(CreateSelfParam(header));

        return Expected
        {
            std::make_shared<const PrototypeSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                header.BodyScope,
                parentTraitName,
                header.Name,
                optTypeName.value(),
                header.Attributes,
                index,
                optSelfParam,
                header.Params,
                header.TypeParams,
                optConstraints.value()
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseTrait(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const TraitSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::Pub },
            SymbolFlags::Generic
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        const auto prototypeScope = scope->CreateChild();

        auto accessModifier = AccessModifier::Priv;
        if (header.ModifierToTokenMap.contains(Modifier::Pub))
        {
            accessModifier = AccessModifier::Pub;
        }

        if (parser.Peek() != TokenKind::TraitKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::TraitKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        std::vector<SymbolName> supertraitNames{};
        if (parser.Peek() == TokenKind::Colon)
        {
            parser.Eat();

            const auto optSupertraitNames = diagnostics.Collect(
                ParseTraitNames(parser, scope)
            );
            if (!optSupertraitNames.has_value())
            {
                return std::move(diagnostics);
            }

            supertraitNames = std::move(optSupertraitNames.value());
        }

        std::vector<std::shared_ptr<const SupertraitSyntax>> supertraits{};
        std::transform(
            begin(supertraitNames),
            end  (supertraitNames),
            back_inserter(supertraits),
            [&](const SymbolName& name)
            {
                return std::make_shared<const SupertraitSyntax>(
                    name,
                    header.Name,
                    header.BodyScope
                );
            }
        );

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        std::vector<std::shared_ptr<const PrototypeSyntax>> prototypes{};
        while (!parser.IsEnd() && (parser.Peek() != TokenKind::CloseBrace))
        {
            if (IsFunctionBegin(parser))
            {
                const auto optPrototype = diagnostics.Collect(ParsePrototype(
                    parser,
                    prototypeScope,
                    CreateName(header),
                    header.TypeParams,
                    prototypes.size()
                ));
                if (optPrototype.has_value())
                {
                    prototypes.push_back(optPrototype.value());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
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

        const auto self = std::make_shared<const TraitSelfSyntax>(
            header.Name.SrcLocation,
            prototypeScope
        );

        std::vector<std::shared_ptr<const TypeReimportSyntax>> typeParamReimports{};
        std::transform(
            begin(header.TypeParams),
            end  (header.TypeParams),
            back_inserter(typeParamReimports),
            [&](const std::shared_ptr<const TypeParamSyntax>& typeParam)
            {
                return std::make_shared<const TypeReimportSyntax>(
                    prototypeScope,
                    header.BodyScope,
                    typeParam->GetName()
                );
            }
        );

        return Expected
        {
            std::make_shared<const TraitSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                header.BodyScope,
                prototypeScope,
                accessModifier,
                header.Name,
                header.Attributes,
                self,
                prototypes,
                header.TypeParams,
                typeParamReimports,
                supertraits
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseField(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& parentStructName,
        const AccessModifier accessModifier,
        const size_t index
    ) -> Expected<std::shared_ptr<const FieldVarSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            {},
            SymbolFlags::None
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        const auto optTypeName = diagnostics.Collect(
            ParseTypeName(parser, scope, RefParsingKind::Forbid)
        );
        if (!optTypeName.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const FieldVarSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                accessModifier,
                parentStructName,
                header.Name,
                optTypeName.value(),
                header.Attributes,
                index
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseStructBody(
        Parser& parser,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& name,
        const AccessModifier memberAccessModifier
    ) -> Expected<std::vector<std::shared_ptr<const FieldVarSyntax>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optFields = diagnostics.Collect(
            ParseList<std::shared_ptr<const FieldVarSyntax>>(
                parser,
                { TokenKind::CloseBrace },
                [&](const size_t index)
                {
                    return ParseField(
                        parser,
                        scope,
                        name,
                        memberAccessModifier,
                        index
                    );
                }
            )
        );
        if (!optFields.has_value())
        {
            return std::move(diagnostics);
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

        return Expected{ optFields.value(), std::move(diagnostics) };
    }

    static auto ParseStruct(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const StructSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::Pub },
            SymbolFlags::Generic
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        auto accessModifier = AccessModifier::Priv;
        if (header.ModifierToTokenMap.contains(Modifier::Pub))
        {
            accessModifier = AccessModifier::Pub;
        }

        auto memberAccessModifier = AccessModifier::Priv;
        if (parser.Peek() == TokenKind::PubKeyword)
        {
            parser.Eat();
            memberAccessModifier = AccessModifier::Pub;
        }

        if (parser.Peek() != TokenKind::StructKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::StructKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        const auto optBlock = diagnostics.Collect(ParseStructBody(
            parser,
            header.BodyScope,
            CreateName(header),
            memberAccessModifier
        ));
        if (!optBlock.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected
        {
            std::make_shared<const StructSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                header.BodyScope,
                accessModifier,
                header.Name,
                header.Attributes,
                optBlock.value(),
                header.TypeParams
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseType(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ISyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (IsTraitBegin(parser))
        {
            const auto optTrait = diagnostics.Collect(
                ParseTrait(parser, scope)
            );
            if (!optTrait.has_value())
            {
                return std::move(diagnostics);
            }

            return Expected{ optTrait.value(), std::move(diagnostics) };
        }
        else if (IsStructBegin(parser))
        {
            const auto optStruct = diagnostics.Collect(
                ParseStruct(parser, scope)
            );
            if (!optStruct.has_value())
            {
                return std::move(diagnostics);
            }

            return Expected{ optStruct.value(), std::move(diagnostics) };
        }

        diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
        return std::move(diagnostics);
    }

    static auto ParseUse(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const UseSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        SymbolName rootTraitName{};

        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (parser.Peek() == TokenKind::ColonColon)
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            parser.Eat();
        }

        const auto optName = diagnostics.Collect(ParseName(parser, scope));
        if (!optName.has_value())
        {
            return std::move(diagnostics);
        }

        rootTraitName.Sections.emplace_back(optName.value());

        while (parser.Peek() == TokenKind::ColonColon)
        {
            parser.Eat();

            const auto optName = diagnostics.Collect(ParseName(parser, scope));
            if (!optName.has_value())
            {
                return std::move(diagnostics);
            }

            rootTraitName.Sections.emplace_back(optName.value());
        }

        return Expected
        {
            std::make_shared<const UseSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                rootTraitName
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseMod(
        Parser& parser,
        const std::shared_ptr<Scope>& scope
    ) -> Expected<std::shared_ptr<const ModSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        const auto optHeader = diagnostics.Collect(ParseNamedSymbolHeader(
            parser,
            scope,
            { Modifier::Pub },
            SymbolFlags::NestedName | SymbolFlags::NamedBodyScope
        ));
        if (!optHeader.has_value())
        {
            return std::move(diagnostics);
        }

        const auto& header = optHeader.value();

        auto accessModifier = AccessModifier::Priv;
        if (header.ModifierToTokenMap.contains(Modifier::Pub))
        {
            accessModifier = AccessModifier::Pub;
        }

        if (parser.Peek() != TokenKind::ModKeyword)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::ModKeyword
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        if (parser.Peek() != TokenKind::OpenBrace)
        {
            diagnostics.Add(CreateUnexpectedTokenError(
                parser.Peek(),
                TokenKind::OpenBrace
            ));
            return std::move(diagnostics);
        }

        parser.Eat();

        std::vector<std::shared_ptr<const ModSyntax>> mods{};
        std::vector<std::shared_ptr<const ISyntax>> types{};
        std::vector<std::shared_ptr<const InherentImplSyntax>> inherentImpls{};
        std::vector<std::shared_ptr<const TraitImplSyntax>> traitImpls{};
        std::vector<std::shared_ptr<const FunctionSyntax>> functions{};
        std::vector<std::shared_ptr<const GlobalVarSyntax>> globalVars{};
        std::vector<std::shared_ptr<const UseSyntax>> uses{};
        while (!parser.IsEnd() && (parser.Peek() != TokenKind::CloseBrace))
        {
            if (IsModBegin(parser))
            {
                const auto optMod = diagnostics.Collect(
                    ParseMod(parser, header.BodyScope)
                );
                if (optMod.has_value())
                {
                    mods.push_back(optMod.value());
                    continue;
                }
            }
            else if (IsTypeBegin(parser))
            {
                const auto optType = diagnostics.Collect(
                    ParseType(parser, header.BodyScope)
                );
                if (optType.has_value())
                {
                    types.push_back(optType.value());
                    continue;
                }
            }
            else if (IsInherentImplBegin(parser))
            {
                const auto optInherentImpl = diagnostics.Collect(
                    ParseInherentImpl(parser, header.BodyScope)
                );
                if (optInherentImpl.has_value())
                {
                    inherentImpls.push_back(optInherentImpl.value());
                    continue;
                }
            }
            else if (IsTraitImplBegin(parser))
            {
                const auto optTraitImpl = diagnostics.Collect(
                    ParseTraitImpl(parser, header.BodyScope)
                );
                if (optTraitImpl.has_value())
                {
                    traitImpls.push_back(optTraitImpl.value());
                    continue;
                }
            }
            else if (IsFunctionBegin(parser))
            {
                const auto optFunction = diagnostics.Collect(
                    ParseFunction(parser, header.BodyScope)
                );
                if (optFunction.has_value())
                {
                    functions.push_back(optFunction.value());
                    continue;
                }
            }
            else if (IsVarBegin(parser))
            {
                const auto optGlobalVar = diagnostics.Collect(
                    ParseGlobalVar(parser, header.BodyScope)
                );
                if (optGlobalVar.has_value())
                {
                    globalVars.push_back(optGlobalVar.value());
                    continue;
                }
            }
            else if (IsUseBegin(parser))
            {
                const auto optUse = diagnostics.Collect(
                    ParseUse(parser, header.BodyScope)
                );
                if (optUse.has_value())
                {
                    uses.push_back(optUse.value());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
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
            std::make_shared<const ModSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                header.BodyScope,
                header.NestedName,
                accessModifier,
                mods,
                types,
                inherentImpls,
                traitImpls,
                functions,
                globalVars,
                uses
            ),
            std::move(diagnostics),
        };
    }

    static auto ParseTopLevelMod(
        Parser& parser,
        const std::string& packageName
    ) -> Expected<std::shared_ptr<const ModSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto beginSrcLocation = parser.GetSrcLocation();

        auto* const compilation = parser.GetFileBuffer()->GetCompilation();

        const auto     scope = compilation->GetGlobalScope();
        const auto bodyScope = scope->GetOrCreateChild(packageName);

        std::vector<std::shared_ptr<const ModSyntax>> mods{};
        std::vector<std::shared_ptr<const ISyntax>> types{};
        std::vector<std::shared_ptr<const InherentImplSyntax>> inherentImpls{};
        std::vector<std::shared_ptr<const TraitImplSyntax>> traitImpls{};
        std::vector<std::shared_ptr<const FunctionSyntax>> functions{};
        std::vector<std::shared_ptr<const GlobalVarSyntax>> globalVars{};
        std::vector<std::shared_ptr<const UseSyntax>> uses{};
        while (!parser.IsEnd())
        {
            if (IsModBegin(parser))
            {
                const auto optMod = diagnostics.Collect(
                    ParseMod(parser, bodyScope)
                );
                if (optMod.has_value())
                {
                    mods.push_back(optMod.value());
                    continue;
                }
            }
            else if (IsTypeBegin(parser))
            {
                const auto optType = diagnostics.Collect(
                    ParseType(parser, bodyScope)
                );
                if (optType.has_value())
                {
                    types.push_back(optType.value());
                    continue;
                }
            }
            else if (IsInherentImplBegin(parser))
            {
                const auto optInherentImpl = diagnostics.Collect(
                    ParseInherentImpl(parser, bodyScope)
                );
                if (optInherentImpl.has_value())
                {
                    inherentImpls.push_back(optInherentImpl.value());
                    continue;
                }
            }
            else if (IsTraitImplBegin(parser))
            {
                const auto optTraitImpl = diagnostics.Collect(
                    ParseTraitImpl(parser, bodyScope)
                );
                if (optTraitImpl.has_value())
                {
                    traitImpls.push_back(optTraitImpl.value());
                    continue;
                }
            }
            else if (IsFunctionBegin(parser))
            {
                const auto optFunction = diagnostics.Collect(
                    ParseFunction(parser, bodyScope)
                );
                if (optFunction.has_value())
                {
                    functions.push_back(optFunction.value());
                    continue;
                }
            }
            else if (IsVarBegin(parser))
            {
                const auto optGlobalVar = diagnostics.Collect(
                    ParseGlobalVar(parser, bodyScope)
                );
                if (optGlobalVar.has_value())
                {
                    globalVars.push_back(optGlobalVar.value());
                    continue;
                }
            }
            else if (IsUseBegin(parser))
            {
                const auto optUse = diagnostics.Collect(
                    ParseUse(parser, bodyScope)
                );
                if (optUse.has_value())
                {
                    uses.push_back(optUse.value());
                    continue;
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedTokenError(parser.Peek()));
            }
            
            parser.DiscardUntil(
                DiscardKind::Inclusive,
                { TokenKind::Semicolon, TokenKind::CloseBrace }
            );
        }

        ACE_ASSERT(parser.IsEnd());

        const std::vector name{ Ident{ beginSrcLocation, packageName } };

        return Expected
        {
            std::make_shared<const ModSyntax>(
                SrcLocation{ beginSrcLocation, parser.GetLastSrcLocation() },
                scope,
                bodyScope,
                name,
                AccessModifier::Pub,
                mods,
                types,
                inherentImpls,
                traitImpls,
                functions,
                globalVars,
                uses
            ),
            std::move(diagnostics),
        };
    }

    auto ParseAST(
        const std::string& packageName,
        const FileBuffer* const fileBuffer
    ) -> Expected<std::shared_ptr<const ModSyntax>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto tokens = diagnostics.Collect(LexTokens(fileBuffer));

        Parser parser{ fileBuffer, std::move(tokens) };

        const auto optMod = diagnostics.Collect(
            ParseTopLevelMod(parser, packageName)
        );
        if (!optMod.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ optMod.value(), std::move(diagnostics) };
    }
}
