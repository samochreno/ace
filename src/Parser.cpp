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
    using ParseIterator = std::vector<std::shared_ptr<const Token>>::const_iterator;

    template<typename T>
    struct ParseResult
    {
        ParseResult(
            const T& t_value,
            const ParseIterator t_endIt
        ) : Value{ t_value },
            EndIterator{ t_endIt }
        {
        }

        T Value{};
        ParseIterator EndIterator{};
    };

    class Parser
    {
    public:
        Parser(
            const std::shared_ptr<Scope>& t_scope,
            const std::vector<std::shared_ptr<const Token>>::const_iterator t_beginIt,
            const std::vector<std::shared_ptr<const Token>>::const_iterator t_endIt
        ) : m_Scope{ t_scope },
            m_BeginIterator{ t_beginIt },
            m_Iterator{ t_beginIt },
            m_EndIterator{ t_endIt }
        {
        }
        ~Parser() = default;

        auto WithScope(const std::shared_ptr<Scope>& t_scope) const -> Parser
        {
            auto parser = *this;
            parser.m_Scope = t_scope;
            return parser;
        }

        auto GetScope() const -> const std::shared_ptr<Scope>&
        {
            return m_Scope;
        }
        auto GetDistance() const -> size_t
        {
            return std::distance(m_BeginIterator, m_Iterator);
        }

        auto IsEnd() const -> bool
        {
            return m_Iterator == m_EndIterator;
        }

        auto Peek() const -> const std::shared_ptr<const Token>&
        {
            return *m_Iterator;
        }

        auto Eat(const size_t t_count = 1) -> void
        {
            m_Iterator += t_count;
        }
        template<typename T>
        auto Eat(const ParseResult<T>& t_result) -> void
        {
            m_Iterator = t_result.EndIterator;
        }

        auto CreateSourceLocation() -> SourceLocation
        {
            return SourceLocation
            {
                Peek()->SourceLocation.Buffer,
                (*m_BeginIterator)->SourceLocation.CharacterBeginIterator,
                (*     m_Iterator)->SourceLocation.CharacterEndIterator,
            };
        }

        template<typename T>
        auto Build(const T& t_value) -> ParseResult<T>
        {
            return ParseResult<T>{ t_value, m_Iterator };
        }

    private:
        std::shared_ptr<Ace::Scope> m_Scope{};
        ParseIterator m_BeginIterator{};
        ParseIterator m_Iterator{};
        ParseIterator m_EndIterator{};
    };

    struct ParseContext
    {
        ParseContext(
            const std::vector<std::shared_ptr<const Token>>::const_iterator t_iterator,
            const std::shared_ptr<Scope>& t_scope
        ) : Iterator{ t_iterator },
            Scope{ t_scope }
        {
        }

        const std::vector<std::shared_ptr<const Token>>::const_iterator Iterator{};
        std::shared_ptr<Ace::Scope> Scope{};
    };

    enum class ReferenceParsingKind
    {
        Allow,
        Disallow,
    };

    enum class OperatorAssociativityKind
    {
        LeftToRight,
        RightToLeft,
    };

    struct OperatorPrecedenceAndAssociativity
    {
        size_t Precedence{};
        OperatorAssociativityKind Associativity{};
    };

    static auto CreateSourceLocationRange(
        const std::vector<std::shared_ptr<const Token>>::const_iterator t_begin,
        const std::vector<std::shared_ptr<const Token>>::const_iterator t_end
    ) -> SourceLocation
    {
        const auto* const sourceBuffer = (*t_begin)->SourceLocation.Buffer;

        const auto& firstToken = *(t_begin);
        const auto&  lastToken = *(t_end - 1);

        return
        {
            sourceBuffer,
            firstToken->SourceLocation.CharacterBeginIterator,
             lastToken->SourceLocation.CharacterEndIterator,
        };
    }

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

    static auto CreateEmptyAttributes() -> std::vector<std::shared_ptr<const AttributeNode>>
    {
        return {};
    }

    static auto GetOperatorFunctionName(
        const std::shared_ptr<const Token>& t_operatorToken,
        const size_t t_paramCount
    ) -> Expected<const char*>
    {
        const auto tokenKind = t_operatorToken->Kind;
        const auto& stringValue = t_operatorToken->String;

        switch (tokenKind)
        {
            case TokenKind::Asterisk:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::Multiplication;
            }

            case TokenKind::Slash:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::Division;
            }

            case TokenKind::Percent:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::Remainder;
            }

            case TokenKind::Plus:
            {
                if (t_paramCount == 1)
                {
                    return SpecialIdentifier::Operator::UnaryPlus;
                }
                else if (t_paramCount == 2)
                {
                    return SpecialIdentifier::Operator::Addition;
                }

                ACE_TRY_UNREACHABLE();
            }

            case TokenKind::Minus:
            {
                if (t_paramCount == 1)
                {
                    return SpecialIdentifier::Operator::UnaryNegation;
                }
                else if (t_paramCount == 2)
                {
                    return SpecialIdentifier::Operator::Subtraction;
                }

                ACE_TRY_UNREACHABLE();
            }

            case TokenKind::LessThan:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::LessThan;
            }

            case TokenKind::GreaterThan:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::GreaterThan;
            }

            case TokenKind::LessThanEquals:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::LessThanEquals;
            }

            case TokenKind::GreaterThanEquals:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::GreaterThanEquals;
            }

            case TokenKind::GreaterThanGreaterThan:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::RightShift;
            }
            
            case TokenKind::LessThanLessThan:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::LeftShift;
            }

            case TokenKind::EqualsEquals:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::Equals;
            }

            case TokenKind::ExclamationEquals:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::NotEquals;
            }

            case TokenKind::Caret:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::XOR;
            }

            case TokenKind::VerticalBar:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::OR;
            }

            case TokenKind::Ampersand:
            {
                ACE_TRY_ASSERT(t_paramCount == 2);
                return SpecialIdentifier::Operator::AND;
            }
            
            case TokenKind::Tilde:
            {
                ACE_TRY_ASSERT(t_paramCount == 1);
                return SpecialIdentifier::Operator::OneComplement;
            }
            
            case TokenKind::ImplKeyword:
            {
                ACE_TRY_ASSERT(t_paramCount == 1);
                return SpecialIdentifier::Operator::ImplicitFrom;
            }
            
            case TokenKind::ExplKeyword:
            {
                ACE_TRY_ASSERT(t_paramCount == 1);
                return SpecialIdentifier::Operator::ExplicitFrom;
            }

            case TokenKind::Identifier:
            {
                if (stringValue == SpecialIdentifier::Copy)
                {
                    ACE_TRY_ASSERT(t_paramCount == 2);
                    return SpecialIdentifier::Operator::Copy;
                }
                
                if (stringValue == SpecialIdentifier::Drop)
                {
                    ACE_TRY_ASSERT(t_paramCount == 1);
                    return SpecialIdentifier::Operator::Drop;
                }

                ACE_TRY_UNREACHABLE();
            }

            default:
            {
                ACE_TRY_UNREACHABLE();
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
        if (t_nameToken == TokenKind::Identifier)
        {
            return Identifier
            {
                t_nameToken->SourceLocation,
                t_nameToken->String,
            };
        }

        ACE_TRY(name, GetOperatorFunctionName(
            t_nameToken,
            t_paramCount
        ));

        ACE_TRY_ASSERT(t_accessModifier == AccessModifier::Public);
        ACE_TRY_ASSERT(!t_hasSelfParam);

        return Identifier
        {
            t_nameToken->SourceLocation,
            name,
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

    static auto ParseExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IExprNode>>>;

    static auto ParsePrimaryExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IExprNode>>>;

    static auto ParseStructConstructionExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const StructConstructionExprNode>>>;

    static auto ParseStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IStmtNode>>>;

    static auto ParseTemplateArgs(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<SymbolName>>>;

    static auto ParseName(
        Parser t_parser
    ) -> Expected<ParseResult<Identifier>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Identifier);
        const auto& nameToken = t_parser.Peek();
        t_parser.Eat();

        return t_parser.Build(Identifier{
            nameToken->SourceLocation,
            nameToken->String,
        });
    }

    static auto ParseNestedName(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<Identifier>>>
    {
        std::vector<Identifier> nestedName{};

        ACE_TRY(name, ParseName(t_parser));
        nestedName.push_back(std::move(name.Value));
        t_parser.Eat(name);

        while (t_parser.Peek() == TokenKind::ColonColon)
        {
            t_parser.Eat();

            ACE_TRY(name, ParseName(t_parser));
            nestedName.push_back(std::move(name.Value));
            t_parser.Eat(name);
        }

        return t_parser.Build(nestedName);
    }

    static auto ParseSymbolNameSection(
        Parser t_parser
    ) -> Expected<ParseResult<SymbolNameSection>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Identifier);
        const std::string& name = t_parser.Peek()->String;
        t_parser.Eat();

        const auto templateArgs = [&]() -> std::vector<SymbolName>
        {
            auto expTemplateArgs = ParseTemplateArgs(t_parser);
            if (!expTemplateArgs)
            {
                return {};
            }

            t_parser.Eat(expTemplateArgs.Unwrap());
            return std::move(expTemplateArgs.Unwrap().Value);
        }();

        return t_parser.Build(SymbolNameSection{
            name,
            templateArgs,
        });
    }

    static auto ParseSymbolName(
        Parser t_parser
    ) -> Expected<ParseResult<SymbolName>>
    {
        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (t_parser.Peek() == TokenKind::ColonColon)
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            t_parser.Eat();
        }
        
        std::vector<SymbolNameSection> sections{};

        ACE_TRY(section, ParseSymbolNameSection(t_parser));
        sections.push_back(std::move(section.Value));
        t_parser.Eat(section);

        while (t_parser.Peek() == TokenKind::ColonColon)
        {
            t_parser.Eat();

            ACE_TRY(section, ParseSymbolNameSection(t_parser));
            sections.push_back(std::move(section.Value));
            t_parser.Eat(section);
        }

        return t_parser.Build(SymbolName{
            sections,
            resolutionScope,
        });
    }

    static auto ParseTypeName(
        Parser t_parser,
        const ReferenceParsingKind t_referenceParsingKind
    ) -> Expected<ParseResult<TypeName>>
    {
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

        ACE_TRY(symbolName, ParseSymbolName(t_parser));
        t_parser.Eat(symbolName);

        return t_parser.Build(TypeName{
            symbolName.Value,
            modifiers,
        });
    }

    static auto ParseTemplateParamNames(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<Identifier>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBracket);
        t_parser.Eat();

        std::vector<Identifier> names{};
        while (t_parser.Peek() != TokenKind::CloseBracket)
        {
            if (!names.empty())
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Comma);
                t_parser.Eat();
            }

            ACE_TRY(name, ParseName(t_parser));
            names.push_back(std::move(name.Value));
            t_parser.Eat(name);
        }

        ACE_TRY_ASSERT(!names.empty());
        t_parser.Eat();

        return t_parser.Build(names);
    }

    static auto ParseImplTemplateParams(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<std::shared_ptr<const ImplTemplateParamNode>>>>
    {
        ACE_TRY(names, ParseTemplateParamNames(t_parser));
        t_parser.Eat(names);
        
        std::vector<std::shared_ptr<const ImplTemplateParamNode>> params{};
        std::transform(
            begin(names.Value),
            end  (names.Value),
            back_inserter(params),
            [&](const Identifier& t_name)
            {
                return std::make_shared<const ImplTemplateParamNode>(
                    t_name.SourceLocation,
                    t_parser.GetScope(),
                    t_name
                );
            }
        );

        return t_parser.Build(params);
    }

    static auto ParseTemplateParams(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>>
    {
        ACE_TRY(names, ParseTemplateParamNames(t_parser));
        t_parser.Eat(names);
        
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> params{};
        std::transform(
            begin(names.Value),
            end  (names.Value),
            back_inserter(params),
            [&](const Identifier& t_name)
            {
                return std::make_shared<const NormalTemplateParamNode>(
                    t_name.SourceLocation,
                    t_parser.GetScope(),
                    t_name
                );
            }
        );

        return t_parser.Build(params);
    }

    static auto ParseTemplateArgs(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<SymbolName>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBracket);
        t_parser.Eat();

        std::vector<SymbolName> args{};
        while (t_parser.Peek() != TokenKind::CloseBracket)
        {
            if (!args.empty())
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Comma);
                t_parser.Eat();
            }

            ACE_TRY(arg, ParseSymbolName(t_parser));
            args.push_back(std::move(arg.Value));
            t_parser.Eat(arg);
        }

        ACE_TRY_ASSERT(!args.empty());
        t_parser.Eat();

        return t_parser.Build(args);
    }

    static auto ParseAttribute(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const AttributeNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBracket);
        t_parser.Eat();

        ACE_TRY(structConstructionExpr, ParseStructConstructionExpr(t_parser));
        t_parser.Eat(structConstructionExpr);

        return t_parser.Build(std::make_shared<const AttributeNode>(
            t_parser.CreateSourceLocation(),
            structConstructionExpr.Value
        ));
    }

    static auto ParseAttributes(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<std::shared_ptr<const AttributeNode>>>>
    {
        std::vector<std::shared_ptr<const AttributeNode>> attributes{};
        while (const auto expAttribute = ParseAttribute(t_parser))
        {
            attributes.push_back(expAttribute.Unwrap().Value);
            t_parser.Eat(expAttribute.Unwrap());
        }

        return t_parser.Build(attributes);
    }

    static auto ParseParam(
        Parser t_parser,
        const size_t t_index
    ) -> Expected<ParseResult<std::shared_ptr<const NormalParamVarNode>>>
    {
        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Allow
        ));
        t_parser.Eat(typeName);

        return t_parser.Build(std::make_shared<const NormalParamVarNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            name.Value,
            typeName.Value,
            attributes.Value,
            t_index
        ));
    }

    static auto ParseParams(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<std::shared_ptr<const NormalParamVarNode>>>>
    {
        std::vector<std::shared_ptr<const NormalParamVarNode>> params{};

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenParen);
        t_parser.Eat();

        bool isFirstParam = true;
        while (t_parser.Peek() != TokenKind::CloseParen)
        {
            if (isFirstParam)
            {
                isFirstParam = false;
            }
            else
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Comma);
                t_parser.Eat();
            }

            ACE_TRY(param, ParseParam(t_parser, params.size()));
            params.push_back(param.Value);
            t_parser.Eat(param);
        }

        t_parser.Eat();

        return t_parser.Build(params);
    }

    static auto ParseArgs(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<std::shared_ptr<const IExprNode>>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenParen);
        t_parser.Eat();

        bool isFirstArg = true;
        std::vector<std::shared_ptr<const IExprNode>> args{};
        while (t_parser.Peek() != TokenKind::CloseParen)
        {
            if (isFirstArg)
            {
                isFirstArg = false;
            }
            else
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Comma);
                t_parser.Eat();
            }

            ACE_TRY(arg, ParseExpr(t_parser));
            args.push_back(arg.Value);
            t_parser.Eat(arg);
        }

        t_parser.Eat();

        return t_parser.Build(args);
    }

    static auto ParseExprExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const ExprExprNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenParen);
        t_parser.Eat();

        ACE_TRY(expr, ParseExpr(t_parser));
        t_parser.Eat(expr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CloseParen);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const ExprExprNode>(
            t_parser.CreateSourceLocation(),
            expr.Value
        ));
    }

    static auto ParseMemberAccessExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const MemberAccessExprNode>>>
    {
        ACE_TRY(expr, ParsePrimaryExpr(t_parser));
        t_parser.Eat(expr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Dot);
        t_parser.Eat();

        ACE_TRY(name, ParseSymbolNameSection(t_parser));
        t_parser.Eat(name);

        return t_parser.Build(std::make_shared<const MemberAccessExprNode>(
            t_parser.CreateSourceLocation(),
            expr.Value,
            name.Value
        ));
    }

    static auto ParseLiteralExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const LiteralExprNode>>>
    {
        const auto& literalToken = t_parser.Peek();

        ACE_TRY(literalKind, [&]() -> Expected<LiteralKind>
        {
            switch (literalToken->Kind)
            {
                case TokenKind::Int8:         return LiteralKind::Int8;
                case TokenKind::Int16:        return LiteralKind::Int16;
                case TokenKind::Int32:        return LiteralKind::Int32;
                case TokenKind::Int64:        return LiteralKind::Int64;
                case TokenKind::UInt8:        return LiteralKind::UInt8;
                case TokenKind::UInt16:       return LiteralKind::UInt16;
                case TokenKind::UInt32:       return LiteralKind::UInt32;
                case TokenKind::UInt64:       return LiteralKind::UInt64;
                case TokenKind::Int:          return LiteralKind::Int;
                case TokenKind::Float32:      return LiteralKind::Float32;
                case TokenKind::Float64:      return LiteralKind::Float64;
                case TokenKind::String:       return LiteralKind::String;
                case TokenKind::TrueKeyword:  return LiteralKind::True;
                case TokenKind::FalseKeyword: return LiteralKind::False;

                default:
                {
                    ACE_TRY_UNREACHABLE();
                };
            }
        }());

        t_parser.Eat();

        return t_parser.Build(std::make_shared<const LiteralExprNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            literalKind,
            literalToken->String
        ));
    }

    static auto ParseLiteralSymbolExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const LiteralSymbolExprNode>>>
    {
        ACE_TRY(name, ParseSymbolName(t_parser));
        t_parser.Eat(name);

        return t_parser.Build(std::make_shared<const LiteralSymbolExprNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            name.Value
        ));
    }

    static auto ParseStructConstructionExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const StructConstructionExprNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == SpecialIdentifier::New);
        t_parser.Eat();

        ACE_TRY(typeName, ParseSymbolName(t_parser));
        t_parser.Eat(typeName);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBrace);
        t_parser.Eat();

        std::vector<StructConstructionExprArg> args{};
        while (t_parser.Peek() != TokenKind::CloseBrace)
        {
            if (!args.empty())
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Comma);
                t_parser.Eat();

                if (t_parser.Peek() == TokenKind::CloseBrace)
                {
                    break;
                }
            }

            ACE_TRY(name, ParseName(t_parser));
            t_parser.Eat(name);

            std::optional<std::shared_ptr<const IExprNode>> optValue{};
            if (t_parser.Peek() == TokenKind::Colon)
            {
                t_parser.Eat();

                ACE_TRY(value, ParseExpr(t_parser));
                optValue = value.Value;
                t_parser.Eat(value);
            }
            
            args.push_back(StructConstructionExprArg{
                name.Value,
                optValue
            });
        }

        t_parser.Eat();

        return t_parser.Build(std::make_shared<const StructConstructionExprNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            typeName.Value,
            std::move(args)
        ));
    }

    static auto ParseCastExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const CastExprNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CastKeyword);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBracket);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Allow
        ));
        t_parser.Eat(typeName);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CloseBracket);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenParen);
        t_parser.Eat();

        ACE_TRY(expr, ParseExpr(t_parser));
        t_parser.Eat(expr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CloseParen);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const CastExprNode>(
            t_parser.CreateSourceLocation(),
            typeName.Value,
            expr.Value
        ));
    }

    static auto ParseAddressOfExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const AddressOfExprNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::AddressOfKeyword);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenParen);
        t_parser.Eat();

        ACE_TRY(expr, ParseExpr(t_parser));
        t_parser.Eat(expr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CloseParen);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const AddressOfExprNode>(
            t_parser.CreateSourceLocation(),
            expr.Value
        ));
    }

    static auto ParseSizeOfExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const SizeOfExprNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::SizeOfKeyword);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBracket);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Allow
        ));
        t_parser.Eat(typeName);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CloseBracket);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const SizeOfExprNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            typeName.Value
        ));
    }

    static auto ParseDerefAsExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const DerefAsExprNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::DerefAsKeyword);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBracket);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Allow
        ));
        t_parser.Eat(typeName);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CloseBracket);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenParen);
        t_parser.Eat();

        ACE_TRY(expr, ParseExpr(t_parser));
        t_parser.Eat(expr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::CloseParen);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const DerefAsExprNode>(
            t_parser.CreateSourceLocation(),
            typeName.Value,
            expr.Value
        ));
    }

    static auto ParseSimpleExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IExprNode>>>
    {
        static constexpr size_t unaryOperatorPrecedenceMin = 1;
        static const auto getUnaryOperatorPrecedence = [](const TokenKind t_operator) -> size_t
        {
            if (t_operator == TokenKind::OpenParen)
            {
                return 0;
            }

            if (
                (t_operator == TokenKind::Plus) ||
                (t_operator == TokenKind::Minus) ||
                (t_operator == TokenKind::Tilde) ||
                (t_operator == TokenKind::Exclamation) ||
                (t_operator == TokenKind::BoxKeyword) ||
                (t_operator == TokenKind::UnboxKeyword)
                )
            {
                return unaryOperatorPrecedenceMin;
            }

            ACE_UNREACHABLE();
        };

        std::vector<std::reference_wrapper<const TokenKind>> prefixOperators{};
        std::vector<std::pair<std::reference_wrapper<const TokenKind>, std::vector<std::shared_ptr<const IExprNode>>>> postfixOperators{};

        while (IsPrefixOperator(t_parser.Peek()->Kind))
        {
            const auto op3rator = t_parser.Peek()->Kind;
            t_parser.Eat();

            prefixOperators.emplace_back(op3rator);
        }

        ACE_TRY(primaryExpr, ParsePrimaryExpr(t_parser));

        ACE_TRY(rootExpr, [&]() -> Expected<std::shared_ptr<const IExprNode>>
        {
            if (const auto expMemberAccessExpr = ParseMemberAccessExpr(t_parser))
            {
                t_parser.Eat(expMemberAccessExpr.Unwrap());
                return std::shared_ptr<const IExprNode>
                {
                    expMemberAccessExpr.Unwrap().Value
                };
            }

            t_parser.Eat(primaryExpr);
            return primaryExpr.Value;
        }());

        while (IsPostfixOperator(t_parser.Peek()->Kind))
        {
            const auto tokenKind = t_parser.Peek()->Kind;

            ACE_TRY(args, [&]() -> Expected<std::vector<std::shared_ptr<const IExprNode>>>
            {
                if (t_parser.Peek() != TokenKind::OpenParen)
                {
                    t_parser.Eat();
                    return std::vector<std::shared_ptr<const IExprNode>>{};
                }

                ACE_TRY(args, ParseArgs(t_parser));
                t_parser.Eat(args);
                return args.Value;
            }());

            postfixOperators.emplace_back(tokenKind, args);
        }

        auto expr = rootExpr;

        const auto collapseLHS = [&]()
        {
            const auto tokenKind = prefixOperators.front();

            const auto collapsedExpr = [&]() -> std::shared_ptr<const IExprNode>
            {
                switch (tokenKind)
                {
                    case TokenKind::Exclamation:
                    {
                        return std::make_shared<const LogicalNegationExprNode>(
                            t_parser.CreateSourceLocation(),
                            expr
                        );
                    }

                    case TokenKind::BoxKeyword:
                    {
                        return std::make_shared<const BoxExprNode>(
                            t_parser.CreateSourceLocation(),
                            expr
                        );
                    }

                    case TokenKind::UnboxKeyword:
                    {
                        return std::make_shared<const UnboxExprNode>(
                            t_parser.CreateSourceLocation(),
                            expr
                        );
                    }

                    default:
                    {
                        return std::make_shared<const UserUnaryExprNode>(
                            t_parser.CreateSourceLocation(),
                            expr, 
                            tokenKind
                        );
                    }
                }
            }();

            expr = collapsedExpr;
            prefixOperators.pop_back();
        };

        const auto collapseRHS = [&]()
        {
            const auto& postfixOperator = postfixOperators.front();
            const auto tokenKind = postfixOperator.first;
            const auto& args = postfixOperator.second;

            const auto collapsedExpr = [&]() -> std::shared_ptr<const IExprNode>
            {
                if (tokenKind == TokenKind::OpenParen)
                {
                    return std::make_shared<const FunctionCallExprNode>(
                        t_parser.CreateSourceLocation(),
                        expr,
                        args
                    );
                }
                else
                {
                    ACE_UNREACHABLE();
                }
            }();

            expr = collapsedExpr;
            postfixOperators.erase(begin(postfixOperators));
        };

        while (!prefixOperators.empty() && !postfixOperators.empty())
        {
            const auto& lhsOperator = prefixOperators.back();
            const auto& rhsOperator = postfixOperators.front().first;

            const auto lhsPrecedence = getUnaryOperatorPrecedence(lhsOperator);
            const auto rhsPrecedence = getUnaryOperatorPrecedence(rhsOperator);

            ACE_ASSERT(lhsPrecedence != rhsPrecedence);

            if (lhsPrecedence < rhsPrecedence)
            {
                collapseLHS();
            }
            else
            {
                collapseRHS();
            }
        }

        while (!prefixOperators.empty())
        {
            collapseLHS();
        }

        while (!postfixOperators.empty())
        {
            collapseRHS();
        }

        return t_parser.Build(expr);
    }

    static auto ParsePrimaryExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IExprNode>>>
    {
        if (const auto expExprExpr = ParseExprExpr(t_parser))
        {
            t_parser.Eat(expExprExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                expExprExpr.Unwrap().Value
            );
        }

        if (const auto expStructConstructionExpr = ParseStructConstructionExpr(t_parser))
        {
            t_parser.Eat(expStructConstructionExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                expStructConstructionExpr.Unwrap().Value
            );
        }

        if (const auto expLiteralExpr = ParseLiteralExpr(t_parser))
        {
            t_parser.Eat(expLiteralExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                expLiteralExpr.Unwrap().Value
            );
        }

        if (const auto expLiteralSymbolExpr = ParseLiteralSymbolExpr(t_parser))
        {
            t_parser.Eat(expLiteralSymbolExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                expLiteralSymbolExpr.Unwrap().Value
            );
        }

        if (const auto castExpr = ParseCastExpr(t_parser))
        {
            t_parser.Eat(castExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                castExpr.Unwrap().Value
            );
        }

        if (const auto addressOfExpr = ParseAddressOfExpr(t_parser))
        {
            t_parser.Eat(addressOfExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                addressOfExpr.Unwrap().Value
            );
        }

        if (const auto sizeOfExpr = ParseSizeOfExpr(t_parser))
        {
            t_parser.Eat(sizeOfExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                sizeOfExpr.Unwrap().Value
            );
        }

        if (const auto derefAsExpr = ParseDerefAsExpr(t_parser))
        {
            t_parser.Eat(derefAsExpr.Unwrap());
            return t_parser.Build<std::shared_ptr<const IExprNode>>(
                derefAsExpr.Unwrap().Value
            );
        }
        
        ACE_TRY_UNREACHABLE();
    }

    static auto ParseExpr(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IExprNode>>>
    {
        static constexpr size_t binaryOperatorPrecedenceMin = 9;
        static const auto getBinaryOperatorPrecedence = [](const TokenKind t_operator) -> size_t
        {
            if (
                (t_operator == TokenKind::Asterisk) ||
                (t_operator == TokenKind::Slash) ||
                (t_operator == TokenKind::Percent)
                )
            {
                return 0;
            }

            if (
                (t_operator == TokenKind::Plus) ||
                (t_operator == TokenKind::Minus)
                )
            {
                return 1;
            }

            if (
                (t_operator == TokenKind::LessThanLessThan) ||
                (t_operator == TokenKind::GreaterThanGreaterThan)
                )
            {
                return 2;
            }

            if (
                (t_operator == TokenKind::LessThan) ||
                (t_operator == TokenKind::LessThanEquals) ||
                (t_operator == TokenKind::GreaterThan) ||
                (t_operator == TokenKind::GreaterThanEquals)
                )
            {
                return 3;
            }

            if (
                (t_operator == TokenKind::EqualsEquals) ||
                (t_operator == TokenKind::ExclamationEquals)
                )
            {
                return 4;
            }

            if (t_operator == TokenKind::Ampersand)
            {
                return 5;
            }

            if (t_operator == TokenKind::Caret)
            {
                return 6;
            }

            if (t_operator == TokenKind::VerticalBar)
            {
                return 7;
            }

            if (t_operator == TokenKind::AmpersandAmpersand)
            {
                return 8;
            }

            if (t_operator == TokenKind::VerticalBarVerticalBar)
            {
                return binaryOperatorPrecedenceMin;
            }

            ACE_UNREACHABLE();
        };
        static const auto getBinaryOperatorPrecedencesAssociativity = [](const size_t t_precedence) -> OperatorAssociativityKind
        {
            switch (t_precedence)
            {
                case 0: return OperatorAssociativityKind::LeftToRight;
                case 1: return OperatorAssociativityKind::LeftToRight;
                case 2: return OperatorAssociativityKind::LeftToRight;
                case 3: return OperatorAssociativityKind::LeftToRight;
                case 4: return OperatorAssociativityKind::LeftToRight;
                case 5: return OperatorAssociativityKind::LeftToRight;
                case 6: return OperatorAssociativityKind::LeftToRight;
                case 7: return OperatorAssociativityKind::LeftToRight;
                case 8: return OperatorAssociativityKind::LeftToRight;
                case 9: return OperatorAssociativityKind::LeftToRight;
            }

            ACE_UNREACHABLE();
        };

        ACE_TRY(simpleExpr, ParseSimpleExpr(t_parser));
        t_parser.Eat(simpleExpr);

        std::vector<std::shared_ptr<const IExprNode>> exprs{};
        std::vector<std::reference_wrapper<const TokenKind>> operators{};

        exprs.push_back(simpleExpr.Value);
        
        while (IsBinaryOperator(t_parser.Peek()->Kind))
        {
            operators.push_back(t_parser.Peek()->Kind);
            t_parser.Eat();

            ACE_TRY(simpleExpr, ParseSimpleExpr(t_parser));
            exprs.push_back(simpleExpr.Value);
            t_parser.Eat(simpleExpr);
        }

        const auto collapseOperator = [&](const size_t t_index)
        {
            const auto& lhsExpr = exprs.at(t_index);
            const auto& rhsExpr = exprs.at(t_index + 1);

            const auto collapsedExpr = [&]() -> std::shared_ptr<const IExprNode>
            {
                const auto tokenKind = operators.at(t_index).get();

                switch (tokenKind)
                {
                    case TokenKind::AmpersandAmpersand:
                    {
                        return std::make_shared<const AndExprNode>(
                            t_parser.CreateSourceLocation(),
                            lhsExpr,
                            rhsExpr
                        );
                    }

                    case TokenKind::VerticalBarVerticalBar:
                    {
                        return std::make_shared<const OrExprNode>(
                            t_parser.CreateSourceLocation(),
                            lhsExpr,
                            rhsExpr
                        );
                    }
                     
                    default:
                    {
                        return std::make_shared<const UserBinaryExprNode>(
                            t_parser.CreateSourceLocation(),
                            lhsExpr,
                            rhsExpr,
                            tokenKind
                        );
                    }
                }
            }();

            operators.erase(begin(operators) + t_index);
            exprs.erase(begin(exprs) + t_index + 1);
            exprs.at(t_index) = collapsedExpr;
        };
        
        for (size_t precendence = 0; precendence <= binaryOperatorPrecedenceMin; precendence++)
        {
            const auto associativity = getBinaryOperatorPrecedencesAssociativity(precendence);

            bool didCollapseAll = false;
            while (!didCollapseAll)
            {
                if (operators.empty())
                {
                    break;
                }

                const auto [startOperatorIndex, endOperatorIndex, operatorIndexIncrement] = [&]() -> std::tuple<int, int, int>
                {
                    switch (associativity)
                    {
                        case OperatorAssociativityKind::LeftToRight:
                        {
                            return
                            {
                                0,
                                static_cast<int>(operators.size()),
                                1,
                            };
                        }

                        case OperatorAssociativityKind::RightToLeft:
                        {
                            return
                            {
                                static_cast<int>(operators.size()) - 1,
                                -1,
                                -1,
                            };
                        }
                    }
                }();

                for (size_t i = startOperatorIndex; i != endOperatorIndex; i += operatorIndexIncrement)
                {
                    const bool isLastIteration = (i == (endOperatorIndex - operatorIndexIncrement));
                    if (isLastIteration)
                    {
                        didCollapseAll = true;
                    }

                    const auto operatorPrecedence = getBinaryOperatorPrecedence(operators.at(i));
                    if (precendence == operatorPrecedence)
                    {
                        collapseOperator(i);
                        break;
                    }
                }
            }
        }

        ACE_TRY_ASSERT((exprs.size() == 1) && (operators.empty()));

        return t_parser.Build(exprs.front());
    }

    static auto ParseBlockStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const BlockStmtNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBrace);
        t_parser.Eat();

        std::vector<std::shared_ptr<const IStmtNode>> stmts{};
        while (t_parser.Peek() != TokenKind::CloseBrace)
        {
            ACE_TRY(stmt, ParseStmt(t_parser.WithScope(scope)));
            stmts.push_back(stmt.Value);
            t_parser.Eat(stmt);
        }

        t_parser.Eat();

        return t_parser.Build(std::make_shared<const BlockStmtNode>(
            t_parser.CreateSourceLocation(),
            scope,
            stmts
        ));
    }

    static auto ParseExprStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const ExprStmtNode>>>
    {
        ACE_TRY(expr, ParseExpr(t_parser));
        t_parser.Eat(expr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const ExprStmtNode>(
            t_parser.CreateSourceLocation(),
            expr.Value
        ));
    }

    static auto ParseAssignmentStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const NormalAssignmentStmtNode>>>
    {
        ACE_TRY(lhsExpr, ParseExpr(t_parser));
        t_parser.Eat(lhsExpr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Equals);
        t_parser.Eat();

        ACE_TRY(rhsExpr, ParseExpr(t_parser));
        t_parser.Eat(rhsExpr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const NormalAssignmentStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            lhsExpr.Value,
            rhsExpr.Value
        ));
    }

    static auto ParseCompoundAssignmentStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const CompoundAssignmentStmtNode>>>
    {
        ACE_TRY(lhsExpr, ParseExpr(t_parser));
        t_parser.Eat(lhsExpr);

        const auto op3rator = t_parser.Peek()->Kind;
        ACE_TRY_ASSERT(IsCompoundAssignmentOperator(op3rator));
        t_parser.Eat();

        ACE_TRY(rhsExpr, ParseExpr(t_parser));
        t_parser.Eat(rhsExpr);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const CompoundAssignmentStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            lhsExpr.Value,
            rhsExpr.Value,
            op3rator
        ));
    }

    static auto ParseVarStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const VarStmtNode>>>
    {
        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Allow
        ));
        t_parser.Eat(typeName);

        ACE_TRY(optAssignment, [&]() -> Expected<std::optional<std::shared_ptr<const IExprNode>>>
        {
            if (t_parser.Peek() == TokenKind::Semicolon)
            {
                t_parser.Eat();
                return std::optional<std::shared_ptr<const IExprNode>>{};
            }
            else if (t_parser.Peek() == TokenKind::Equals)
            {
                t_parser.Eat();

                ACE_TRY(expr, ParseExpr(t_parser));
                t_parser.Eat(expr);

                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
                t_parser.Eat();

                return std::optional{ expr.Value };
            }

            ACE_TRY_UNREACHABLE();
        }());

        return t_parser.Build(std::make_shared<const VarStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            name.Value,
            typeName.Value,
            optAssignment
        ));
    }

    static auto ParseIfBlock(
        Parser t_parser
    ) -> Expected<ParseResult<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::IfKeyword);
        t_parser.Eat();

        ACE_TRY(condition, ParseExpr(t_parser));
        t_parser.Eat(condition);

        ACE_TRY(body, ParseBlockStmt(t_parser));
        t_parser.Eat(body);

        return t_parser.Build(std::pair{
            condition.Value,
            body.Value
        });
    }

    static auto ParseElifBlock(
        Parser t_parser
    ) -> Expected<ParseResult<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::ElifKeyword);
        t_parser.Eat();

        ACE_TRY(condition, ParseExpr(t_parser));
        t_parser.Eat(condition);

        ACE_TRY(body, ParseBlockStmt(t_parser));
        t_parser.Eat(body);

        return t_parser.Build(std::pair{
            condition.Value,
            body.Value
        });
    }

    static auto ParseElseBlock(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const BlockStmtNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::ElseKeyword);
        t_parser.Eat();

        ACE_TRY(body, ParseBlockStmt(t_parser));
        t_parser.Eat(body);

        return t_parser.Build(body.Value);
    }

    static auto ParseIfStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IfStmtNode>>>
    {
        std::vector<std::shared_ptr<const IExprNode>> conditions{};
        std::vector<std::shared_ptr<const BlockStmtNode>> bodies{};

        ACE_TRY(ifBlock, ParseIfBlock(t_parser));
        conditions.push_back(ifBlock.Value.first);
        bodies.push_back(ifBlock.Value.second);
        t_parser.Eat(ifBlock);

        while (t_parser.Peek() == TokenKind::ElifKeyword)
        {
            ACE_TRY(elifBlock, ParseElifBlock(t_parser));
            conditions.push_back(elifBlock.Value.first);
            bodies.push_back(elifBlock.Value.second);
            t_parser.Eat(elifBlock);
        }

        if (const auto expElseBlock = ParseElseBlock(t_parser))
        {
            bodies.push_back(expElseBlock.Unwrap().Value);
            t_parser.Eat(expElseBlock.Unwrap());
        }

        return t_parser.Build(std::make_shared<const IfStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            conditions,
            bodies
        ));
    }

    static auto ParseWhileStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const WhileStmtNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::WhileKeyword);
        t_parser.Eat();

        ACE_TRY(condition, ParseExpr(t_parser));
        t_parser.Eat(condition);

        ACE_TRY(body, ParseBlockStmt(t_parser));
        t_parser.Eat(body);

        return t_parser.Build(std::make_shared<const WhileStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            condition.Value,
            body.Value
        ));
    }

    static auto ParseReturnStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const ReturnStmtNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::ReturnKeyword);
        t_parser.Eat();

        ACE_TRY(optExpr, [&]() -> Expected<std::optional<std::shared_ptr<const IExprNode>>>
        {
            if (t_parser.Peek() == TokenKind::Semicolon)
            {
                t_parser.Eat();
                return std::optional<std::shared_ptr<const IExprNode>>{};
            }

            ACE_TRY(expr, ParseExpr(t_parser));
            t_parser.Eat(expr);

            ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
            t_parser.Eat();

            return std::optional{ expr.Value };
        }());

        return t_parser.Build(std::make_shared<const ReturnStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            optExpr
        ));
    }

    static auto ParseExitStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const ExitStmtNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::ExitKeyword);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const ExitStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope()
        ));
    }

    static auto ParseAssertStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const AssertStmtNode>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::AssertKeyword);
        t_parser.Eat();

        ACE_TRY(condition, ParseExpr(t_parser));
        t_parser.Eat(condition);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const AssertStmtNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            condition.Value
        ));
    }

    static auto ParseKeywordStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IStmtNode>>>
    {
        if (const auto expIfStmt = ParseIfStmt(t_parser))
        {
            t_parser.Eat(expIfStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expIfStmt.Unwrap().Value
            );
        }

        if (const auto expWhileStmt = ParseWhileStmt(t_parser))
        {
            t_parser.Eat(expWhileStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expWhileStmt.Unwrap().Value
            );
        }

        if (const auto expReturnStmt = ParseReturnStmt(t_parser))
        {
            t_parser.Eat(expReturnStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expReturnStmt.Unwrap().Value
            );
        }

        if (const auto expExitStmt = ParseExitStmt(t_parser))
        {
            t_parser.Eat(expExitStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expExitStmt.Unwrap().Value
            );
        }

        if (const auto expAssertStmt = ParseAssertStmt(t_parser))
        {
            t_parser.Eat(expAssertStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expAssertStmt.Unwrap().Value
            );
        }

        ACE_TRY_UNREACHABLE();
    }

    static auto ParseStmt(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const IStmtNode>>>
    {
        if (const auto expExprStatemment = ParseExprStmt(t_parser))
        {
            t_parser.Eat(expExprStatemment.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expExprStatemment.Unwrap().Value
            );
        }

        if (const auto expAssignmentStmt = ParseAssignmentStmt(t_parser))
        {
            t_parser.Eat(expAssignmentStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expAssignmentStmt.Unwrap().Value
            );
        }

        if (const auto expCompoundAssignmentStmt = ParseCompoundAssignmentStmt(t_parser))
        {
            t_parser.Eat(expCompoundAssignmentStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expCompoundAssignmentStmt.Unwrap().Value
            );
        }

        if (const auto expVarStmt = ParseVarStmt(t_parser))
        {
            t_parser.Eat(expVarStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expVarStmt.Unwrap().Value
            );
        }

        if (const auto expKeywordStmt = ParseKeywordStmt(t_parser))
        {
            t_parser.Eat(expKeywordStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expKeywordStmt.Unwrap().Value
            );
        }

        if (const auto expBlockStmt = ParseBlockStmt(t_parser))
        {
            t_parser.Eat(expBlockStmt.Unwrap());
            return t_parser.Build<std::shared_ptr<const IStmtNode>>(
                expBlockStmt.Unwrap().Value
            );
        }

        ACE_TRY_UNREACHABLE();
    }

    static auto ParseFunctionOrOperatorNameToken(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const Token>>>
    {
        if (t_parser.Peek() == TokenKind::Identifier)
        {
            const auto& nameToken = t_parser.Peek();
            t_parser.Eat();
            return t_parser.Build(nameToken);
        }

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OperatorKeyword);
        t_parser.Eat();

        const auto& operatorToken = t_parser.Peek();
        if (!IsUserOperator(operatorToken->Kind))
        {
            ACE_TRY_ASSERT(
                operatorToken == TokenKind::ImplKeyword ||
                operatorToken == TokenKind::ExplKeyword ||
                operatorToken == TokenKind::Identifier
            );
        }

        t_parser.Eat();
        
        return t_parser.Build(operatorToken);
    }

    static auto ParseImplFunction(
        Parser t_parser,
        const SymbolName& t_selfTypeName
    ) -> Expected<ParseResult<std::shared_ptr<const FunctionNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(nameToken, ParseFunctionOrOperatorNameToken(t_parser));
        t_parser.Eat(nameToken);

        ACE_TRY(params, ParseParams(t_parser.WithScope(scope)));
        t_parser.Eat(params);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Disallow
        ));
        t_parser.Eat(typeName);

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};
        bool hasExternModifier = false;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            if (t_parser.Peek() == TokenKind::Identifier)
            {
                ACE_TRY_ASSERT(t_parser.Peek() == SpecialIdentifier::Self);
                optSelfToken = t_parser.Peek();
                t_parser.Eat();
            }

            if (t_parser.Peek() == TokenKind::ExternKeyword)
            {
                hasExternModifier = true;
                ACE_TRY_ASSERT(!optSelfToken.has_value());
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY(name, CreateFunctionOrOperatorName(
            nameToken.Value,
            params.Value.size(),
            accessModifier,
            optSelfToken.has_value()
        ));

        ACE_TRY(optBody, [&]() -> Expected<std::optional<std::shared_ptr<const BlockStmtNode>>>
        {
            if (hasExternModifier)
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
                t_parser.Eat();

                return std::optional<std::shared_ptr<const BlockStmtNode>>{};
            }
            else
            {
                ACE_TRY(body, ParseBlockStmt(t_parser.WithScope(scope)));
                t_parser.Eat(body);

                return std::optional{ body.Value };
            }
        }());

        const auto optSelfParam = CreateSelfParam(
            optSelfToken,
            scope,
            t_selfTypeName
        );

        return t_parser.Build(std::make_shared<const FunctionNode>(
            t_parser.CreateSourceLocation(),
            scope,
            name,
            typeName.Value,
            attributes.Value,
            accessModifier,
            optSelfParam,
            params.Value,
            optBody
        ));
    }

    static auto ParseImplFunctionTemplate(
        Parser t_parser,
        const SymbolName& t_selfTypeName
    ) -> Expected<ParseResult<std::shared_ptr<const FunctionTemplateNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY(templateParams, ParseTemplateParams(
            t_parser.WithScope(scope)
        ));
        t_parser.Eat(templateParams);

        ACE_TRY(params, ParseParams(t_parser.WithScope(scope)));
        t_parser.Eat(params);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Disallow
        ));
        t_parser.Eat(typeName);

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            if (t_parser.Peek() == TokenKind::Identifier)
            {
                ACE_TRY_ASSERT(t_parser.Peek() == SpecialIdentifier::Self);
                optSelfToken = t_parser.Peek();
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY(body, ParseBlockStmt(t_parser.WithScope(scope)));
        t_parser.Eat(body);

        const auto selfParam = [&]() -> std::optional<std::shared_ptr<const SelfParamVarNode>>
        {
            if (!optSelfToken.has_value())
            {
                return std::nullopt;
            }

            return std::make_shared<const SelfParamVarNode>(
                optSelfToken.value()->SourceLocation,
                scope,
                t_selfTypeName
            );
        }();

        const auto function = std::make_shared<const FunctionNode>(
            t_parser.CreateSourceLocation(),
            scope,
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            selfParam,
            params.Value,
            body.Value
        );

        return t_parser.Build(std::make_shared<const FunctionTemplateNode>(
            t_parser.CreateSourceLocation(),
            std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
            templateParams.Value,
            function
        ));
    }

    static auto ParseImpl(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const ImplNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::ImplKeyword);
        t_parser.Eat();

        ACE_TRY(typeName, ParseSymbolName(t_parser));

        // TODO: Remove this block after impl template specialization.
        {
            const bool foundTemplatedSection = std::find_if(
                begin(typeName.Value.Sections),
                end  (typeName.Value.Sections),
                [](const SymbolNameSection& t_section)
                {
                    return t_section.TemplateArgs.empty();
                }
            ) == end(typeName.Value.Sections);

            ACE_TRY_ASSERT(!foundTemplatedSection);
        }

        t_parser.Eat(typeName);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBrace);
        t_parser.Eat();

        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};

        while (t_parser.Peek() != TokenKind::CloseBrace)
        {
            if (const auto expFunction = ParseImplFunction(t_parser.WithScope(scope), typeName.Value))
            {
                functions.push_back(expFunction.Unwrap().Value);
                t_parser.Eat(expFunction.Unwrap());
                continue;
            }

            if (const auto expFunctionTemplate = ParseImplFunctionTemplate(t_parser.WithScope(scope), typeName.Value))
            {
                functionTemplates.push_back(expFunctionTemplate.Unwrap().Value);
                t_parser.Eat(expFunctionTemplate.Unwrap());
                continue;
            }

            ACE_TRY_UNREACHABLE();
        }

        t_parser.Eat();

        return t_parser.Build(std::make_shared<const ImplNode>(
            t_parser.CreateSourceLocation(),
            scope,
            typeName.Value,
            functions,
            functionTemplates
        ));
    }

    static auto ParseTemplatedImplFunction(
        Parser t_parser,
        const SymbolName& t_selfTypeName,
        const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& t_implTemplateParams
    ) -> Expected<ParseResult<std::shared_ptr<const FunctionTemplateNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(nameToken, ParseFunctionOrOperatorNameToken(t_parser));
        t_parser.Eat(nameToken);

        ACE_TRY(templateParams, [&]() -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
        {
            if (const auto expTemplateParams = ParseTemplateParams(t_parser.WithScope(scope)))
            {
                t_parser.Eat(expTemplateParams.Unwrap());
                return expTemplateParams.Unwrap().Value;
            }
            else
            {
                return std::vector<std::shared_ptr<const NormalTemplateParamNode>>{};
            }
        }());

        ACE_TRY(params, ParseParams(t_parser.WithScope(scope)));
        t_parser.Eat(params);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Disallow
        ));
        t_parser.Eat(typeName);

        auto accessModifier = AccessModifier::Private;
        std::optional<std::shared_ptr<const Token>> optSelfToken{};

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            if (t_parser.Peek() == TokenKind::Identifier)
            {
                ACE_TRY_ASSERT(t_parser.Peek() == SpecialIdentifier::Self);
                optSelfToken = t_parser.Peek();
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY(name, CreateFunctionOrOperatorName(
            nameToken.Value,
            params.Value.size(),
            accessModifier,
            optSelfToken.has_value()
        ));

        ACE_TRY(body, ParseBlockStmt(t_parser));
        t_parser.Eat(body);

        const auto optSelfParam = CreateSelfParam(
            optSelfToken,
            scope,
            t_selfTypeName
        );

        const auto function = std::make_shared<const FunctionNode>(
            t_parser.CreateSourceLocation(),
            scope,
            name,
            typeName.Value,
            attributes.Value,
            accessModifier,
            optSelfParam,
            params.Value,
            body.Value
        );

        std::vector<std::shared_ptr<const ImplTemplateParamNode>> clonedImplTemplateParams{};
        std::transform(
            begin(t_implTemplateParams),
            end  (t_implTemplateParams),
            back_inserter(clonedImplTemplateParams),
            [&](const std::shared_ptr<const ImplTemplateParamNode>& t_implTemplateParam)
            {
                return t_implTemplateParam->CloneInScope(scope);
            }
        );

        return t_parser.Build(std::make_shared<const FunctionTemplateNode>(
            t_parser.CreateSourceLocation(),
            clonedImplTemplateParams,
            templateParams,
            function
        ));
    }

    static auto ParseTemplatedImpl(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const TemplatedImplNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::ImplKeyword);
        t_parser.Eat();

        ACE_TRY(templateParams, ParseImplTemplateParams(
            t_parser.WithScope(scope)
        ));
        t_parser.Eat(templateParams);

        ACE_TRY(typeName, ParseSymbolName(t_parser));

        // TODO: Remove this block after impl template specialization.
        {
            const bool foundTemplatedSection = std::find_if(
                begin(typeName.Value.Sections),
                end  (typeName.Value.Sections) - 1,
                [](const SymbolNameSection& t_section)
                {
                    return t_section.TemplateArgs.empty();
                }
            ) == end(typeName.Value.Sections);

            ACE_TRY_ASSERT(!foundTemplatedSection);

            const auto& templateArgs =
                typeName.Value.Sections.back().TemplateArgs;
            ACE_TRY_ASSERT(templateParams.Value.size() == templateArgs.size());

            std::unordered_set<std::string> templateParamSet{};
            ACE_TRY_VOID(TransformExpectedVector(templateParams.Value,
            [&](const std::shared_ptr<const ImplTemplateParamNode>& t_templateParam) -> Expected<void>
            {
                const std::string& templateParamName =
                    t_templateParam->GetName().String;

                ACE_TRY_ASSERT(!templateParamSet.contains(templateParamName));
                templateParamSet.insert(templateParamName);

                return Void{};
            }));
            
            ACE_TRY_VOID(TransformExpectedVector(templateArgs,
            [&](const SymbolName& t_arg) -> Expected<void>
            {
                ACE_TRY_ASSERT(t_arg.Sections.size() == 1);
                ACE_TRY_ASSERT(t_arg.Sections.back().TemplateArgs.empty());

                const std::string& templateArgName =
                    t_arg.Sections.front().Name;
                ACE_TRY_ASSERT(templateParamSet.contains(templateArgName));
                templateParamSet.erase(templateArgName);

                return Void{};
            }));
        }

        t_parser.Eat(typeName);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBrace);
        t_parser.Eat();

        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};

        while (t_parser.Peek() != TokenKind::CloseBrace)
        {
            if (const auto expFunctionTemplate = ParseTemplatedImplFunction(t_parser.WithScope(scope), typeName.Value, templateParams.Value))
            {
                functionTemplates.push_back(expFunctionTemplate.Unwrap().Value);
                t_parser.Eat(expFunctionTemplate.Unwrap());
                continue;
            }

            ACE_TRY_UNREACHABLE();
        }

        t_parser.Eat();

        auto typeTemplateName = typeName.Value;
        auto& typeTemplateNameLastSection = typeTemplateName.Sections.back();
        typeTemplateNameLastSection.TemplateArgs.clear();
        typeTemplateNameLastSection.Name = SpecialIdentifier::CreateTemplate(
            typeTemplateNameLastSection.Name
        );

        return t_parser.Build(std::make_shared<const TemplatedImplNode>(
            t_parser.CreateSourceLocation(),
            scope,
            typeTemplateName,
            std::vector<std::shared_ptr<const FunctionNode>>{},
            functionTemplates
        ));
    }

    static auto ParseFunction(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const FunctionNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY(params, ParseParams(t_parser.WithScope(scope)));
        t_parser.Eat(params);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Disallow
        ));
        t_parser.Eat(typeName);

        auto accessModifier = AccessModifier::Private;
        bool hasExternModifier = false;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            if (t_parser.Peek() == TokenKind::ExternKeyword)
            {
                hasExternModifier = true;
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY(optBody, [&]() -> Expected<std::optional<std::shared_ptr<const BlockStmtNode>>>
        {
            if (hasExternModifier)
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
                t_parser.Eat();

                return std::optional<std::shared_ptr<const BlockStmtNode>>{};
            }
            else
            {
                ACE_TRY(body, ParseBlockStmt(t_parser.WithScope(scope)));
                t_parser.Eat(body);

                return std::optional{ body.Value };
            }
        }());

        return t_parser.Build(std::make_shared<const FunctionNode>(
            t_parser.CreateSourceLocation(),
            scope,
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            std::nullopt,
            params.Value,
            optBody
        ));
    }

    static auto ParseFunctionTemplate(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const FunctionTemplateNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY(templateParams, ParseTemplateParams(t_parser.WithScope(scope)));
        t_parser.Eat(templateParams);

        ACE_TRY(params, ParseParams(t_parser.WithScope(scope)));
        t_parser.Eat(params);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Disallow
        ));
        t_parser.Eat(typeName);

        auto accessModifier = AccessModifier::Private;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY(body, ParseBlockStmt(t_parser.WithScope(scope)));
        t_parser.Eat(body);

        const auto function = std::make_shared<const FunctionNode>(
            t_parser.CreateSourceLocation(),
            scope,
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            std::nullopt,
            params.Value,
            body.Value
        );

        return t_parser.Build(std::make_shared<const FunctionTemplateNode>(
            t_parser.CreateSourceLocation(),
            std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
            templateParams.Value,
            function
        ));
    }

    static auto ParseVar(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const StaticVarNode>>>
    {
        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Disallow
        ));
        t_parser.Eat(typeName);

        auto accessModifier = AccessModifier::Private;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Semicolon);
        t_parser.Eat();

        return t_parser.Build(std::make_shared<const StaticVarNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier
        ));
    }

    static auto ParseMemberVar(
        Parser t_parser,
        const size_t t_index
    ) -> Expected<ParseResult<std::shared_ptr<const InstanceVarNode>>>
    {
        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY(typeName, ParseTypeName(
            t_parser,
            ReferenceParsingKind::Disallow
        ));
        t_parser.Eat(typeName);

        auto accessModifier = AccessModifier::Private;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        return t_parser.Build(std::make_shared<const InstanceVarNode>(
            t_parser.CreateSourceLocation(),
            t_parser.GetScope(),
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            t_index
        ));
    }

    static auto ParseStructBody(
        Parser t_parser
    ) -> Expected<ParseResult<std::vector<std::shared_ptr<const InstanceVarNode>>>>
    {
        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBrace);
        t_parser.Eat();

        std::vector<std::shared_ptr<const InstanceVarNode>> variables{};
        while (t_parser.Peek() != TokenKind::CloseBrace)
        {
            if (!variables.empty())
            {
                ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Comma);
                t_parser.Eat();

                if (t_parser.Peek() == TokenKind::CloseBrace)
                {
                    break;
                }
            }

            ACE_TRY(variable, ParseMemberVar(t_parser, variables.size()));
            variables.push_back(variable.Value);
            t_parser.Eat(variable);
        }

        t_parser.Eat();

        return t_parser.Build(variables);
    }

    static auto ParseStruct(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const StructTypeNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::StructKeyword);
        t_parser.Eat();

        auto accessModifier = AccessModifier::Private;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY(body, ParseStructBody(t_parser.WithScope(scope)));
        t_parser.Eat(body);

        return t_parser.Build(std::make_shared<const StructTypeNode>(
            t_parser.CreateSourceLocation(),
            scope,
            name.Value,
            attributes.Value,
            accessModifier,
            body.Value
        ));
    }

    static auto ParseStructTemplate(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const TypeTemplateNode>>>
    {
        const auto scope = t_parser.GetScope()->GetOrCreateChild({});

        ACE_TRY(attributes, ParseAttributes(t_parser));
        t_parser.Eat(attributes);

        ACE_TRY(name, ParseName(t_parser));
        t_parser.Eat(name);

        ACE_TRY(templateParams, ParseTemplateParams(
            t_parser.WithScope(scope)
        ));
        t_parser.Eat(templateParams);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::StructKeyword);
        t_parser.Eat();

        auto accessModifier = AccessModifier::Private;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto startDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != startDistance);
        }

        ACE_TRY(body, ParseStructBody(t_parser.WithScope(scope)));
        t_parser.Eat(body);

        const auto type = std::make_shared<const StructTypeNode>(
            t_parser.CreateSourceLocation(),
            scope,
            name.Value,
            attributes.Value,
            accessModifier,
            body.Value
        );

        return t_parser.Build(std::make_shared<const TypeTemplateNode>(
            t_parser.CreateSourceLocation(),
            templateParams.Value,
            type
        ));
    }

    static auto ParseTypeTemplate(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const TypeTemplateNode>>>
    {
        if (const auto expStructTemplate = ParseStructTemplate(t_parser))
        {
            return expStructTemplate;
        }

        ACE_TRY_UNREACHABLE();
    }

    static auto ParseType(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const ITypeNode>>>
    {
        if (const auto expStruct = ParseStruct(t_parser))
        {
            t_parser.Eat(expStruct.Unwrap());
            return t_parser.Build<std::shared_ptr<const ITypeNode>>(
                expStruct.Unwrap().Value
            );
        }

        ACE_TRY_UNREACHABLE();
    }

    static auto ParseModule(
        Parser t_parser
    ) -> Expected<ParseResult<std::shared_ptr<const ModuleNode>>>
    {
        ACE_TRY(name, ParseNestedName(t_parser));
        t_parser.Eat(name);

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::Colon);
        t_parser.Eat();

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::ModuleKeyword);
        t_parser.Eat();

        auto accessModifier = AccessModifier::Private;

        if (t_parser.Peek() == TokenKind::MinusGreaterThan)
        {
            t_parser.Eat();
            const auto originalDistance = t_parser.GetDistance();

            if (t_parser.Peek() == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                t_parser.Eat();
            }

            ACE_TRY_ASSERT(t_parser.GetDistance() != originalDistance);
        }

        std::vector<std::shared_ptr<Scope>> scopes{};
        scopes.push_back(t_parser.GetScope());
        std::transform(
            begin(name.Value),
            end  (name.Value),
            back_inserter(scopes),
            [&](const Identifier& t_name)
            {
                return scopes.back()->GetOrCreateChild(t_name.String);
            }
        );

        ACE_TRY_ASSERT(t_parser.Peek() == TokenKind::OpenBrace);
        t_parser.Eat();

        std::vector<std::shared_ptr<const ModuleNode>> modules{};
        std::vector<std::shared_ptr<const ITypeNode>> types{};
        std::vector<std::shared_ptr<const TypeTemplateNode>> typeTemplates{};
        std::vector<std::shared_ptr<const ImplNode>> impls{};
        std::vector<std::shared_ptr<const TemplatedImplNode>> templatedImpls{};
        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        std::vector<std::shared_ptr<const StaticVarNode>> variables{};

        while (t_parser.Peek() != TokenKind::CloseBrace)
        {
            const auto selfScope = scopes.back();

            if (const auto expModule = ParseModule(t_parser.WithScope(selfScope)))
            {
                modules.push_back(expModule.Unwrap().Value);
                t_parser.Eat(expModule.Unwrap());
                continue;
            }

            if (const auto expType = ParseType(t_parser.WithScope(selfScope)))
            {
                types.push_back(expType.Unwrap().Value);
                t_parser.Eat(expType.Unwrap());
                continue;
            }

            if (const auto expTypeTemplate = ParseTypeTemplate(t_parser.WithScope(selfScope)))
            {
                typeTemplates.push_back(expTypeTemplate.Unwrap().Value);
                t_parser.Eat(expTypeTemplate.Unwrap());
                continue;
            }

            if (const auto expImpl = ParseImpl(t_parser.WithScope(selfScope)))
            {
                impls.push_back(expImpl.Unwrap().Value);
                t_parser.Eat(expImpl.Unwrap());
                continue;
            }

            if (const auto expTemplatedImpl = ParseTemplatedImpl(t_parser.WithScope(selfScope)))
            {
                templatedImpls.push_back(expTemplatedImpl.Unwrap().Value);
                t_parser.Eat(expTemplatedImpl.Unwrap());
                continue;
            }

            if (const auto expFunction = ParseFunction(t_parser.WithScope(selfScope)))
            {
                functions.push_back(expFunction.Unwrap().Value);
                t_parser.Eat(expFunction.Unwrap());
                continue;
            }

            if (const auto expFunctionTemplate = ParseFunctionTemplate(t_parser.WithScope(selfScope)))
            {
                functionTemplates.push_back(expFunctionTemplate.Unwrap().Value);
                t_parser.Eat(expFunctionTemplate.Unwrap());
                continue;
            }

            if (const auto expVar = ParseVar(t_parser.WithScope(selfScope)))
            {
                variables.push_back(expVar.Unwrap().Value);
                t_parser.Eat(expVar.Unwrap());
                continue;
            }

            ACE_TRY_UNREACHABLE();
        }

        t_parser.Eat();

        return t_parser.Build(std::make_shared<const ModuleNode>(
            t_parser.CreateSourceLocation(),
            scopes.front(),
            scopes.back(),
            name.Value,
            accessModifier,
            modules,
            types,
            typeTemplates,
            impls,
            templatedImpls,
            functions,
            functionTemplates,
            variables
        ));
    }

    auto ParseAST(
        const FileBuffer* const t_fileBuffer,
        const std::vector<std::shared_ptr<const Token>>& t_tokens
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        std::vector<std::shared_ptr<const Token>> tokens{};

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

        tokens.emplace_back(std::make_shared<const Token>(
            leadingSourceLocation,
            TokenKind::Identifier,
            t_fileBuffer->GetCompilation()->Package.Name
        ));
        tokens.emplace_back(std::make_shared<const Token>(
            leadingSourceLocation,
            TokenKind::Colon
        ));
        tokens.emplace_back(std::make_shared<const Token>(
            leadingSourceLocation,
            TokenKind::ModuleKeyword
        ));
        tokens.emplace_back(std::make_shared<const Token>(
            leadingSourceLocation,
            TokenKind::OpenBrace
        ));

        std::transform(
            begin(t_tokens),
            end  (t_tokens),
            back_inserter(tokens),
            [](const std::shared_ptr<const Token>& t_token)
            {
                return t_token;
            }
        );

        tokens.insert(
            end(tokens) - 1,
            std::make_shared<const Token>(
                trailingSourceLocation,
                TokenKind::CloseBrace
            )
        );

        Parser parser
        {
            t_fileBuffer->GetCompilation()->GlobalScope.Unwrap(),
            begin(tokens),
            end  (tokens),
        };

        ACE_TRY(module, ParseModule(parser));
        parser.Eat(module);

        ACE_TRY_ASSERT(parser.Peek() == TokenKind::EndOfFile);
        parser.Eat();

        ACE_ASSERT(parser.IsEnd());

        return module.Value;
    }
}
