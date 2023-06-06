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

#include "Token.hpp"
#include "Node/All.hpp"
#include "Diagnostics.hpp"
#include "Asserts.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"
#include "ParseData.hpp"
#include "Scope.hpp"
#include "Utility.hpp"
#include "Name.hpp"

namespace Ace::Parsing
{
    enum class OperatorAssociativity
    {
        LeftToRight,
        RightToLeft,
    };

    struct OperatorPrecedenceAndAssociativity
    {
        size_t Precedence{};
        OperatorAssociativity Associativity{};
    };

    namespace OperatorTokenMask
    {
        static const TokenKind::Set CompoundAssignment = TokenKind::New()
            .set(TokenKind::PlusEquals)
            .set(TokenKind::MinusEquals)
            .set(TokenKind::AsteriskEquals)
            .set(TokenKind::SlashEquals)
            .set(TokenKind::PercentEquals)
            .set(TokenKind::LessThanLessThanEquals)
            .set(TokenKind::GreaterThanGreaterThanEquals)
            .set(TokenKind::CaretEquals)
            .set(TokenKind::VerticalBarEquals)
            .set(TokenKind::AmpersandEquals);

        namespace Prefix
        {
            static const TokenKind::Set User = TokenKind::New()
                .set(TokenKind::Plus)
                .set(TokenKind::Minus)
                .set(TokenKind::Tilde);

            static const TokenKind::Set All = OperatorTokenMask::Prefix::User | TokenKind::New()
                .set(TokenKind::Exclamation)
                .set(TokenKind::BoxKeyword)
                .set(TokenKind::UnboxKeyword);
        }

        static const TokenKind::Set Postfix = TokenKind::New()
            .set(TokenKind::OpenParen);

        namespace Binary
        {
            static const TokenKind::Set User = TokenKind::New()
                .set(TokenKind::Asterisk)
                .set(TokenKind::Slash)
                .set(TokenKind::Percent)
                .set(TokenKind::Plus)
                .set(TokenKind::Minus)
                .set(TokenKind::LessThan)
                .set(TokenKind::GreaterThan)
                .set(TokenKind::LessThanEquals)
                .set(TokenKind::GreaterThanEquals)
                .set(TokenKind::LessThanLessThan)
                .set(TokenKind::GreaterThanGreaterThan)
                .set(TokenKind::EqualsEquals)
                .set(TokenKind::ExclamationEquals)
                .set(TokenKind::Caret)
                .set(TokenKind::VerticalBar)
                .set(TokenKind::Ampersand);

            static const TokenKind::Set All = OperatorTokenMask::Binary::User | TokenKind::New()
                .set(TokenKind::VerticalBarVerticalBar)
                .set(TokenKind::AmpersandAmpersand);
        }

        static const TokenKind::Set User = OperatorTokenMask::Prefix::User | OperatorTokenMask::Binary::User;
    }

    auto Parser::ParseAST(
        const Compilation* const t_compilation, 
        std::vector<Token>&& t_tokens
    ) -> Expected<std::shared_ptr<const Node::Module>>
    {
        std::vector<Token> leadingTokens{};
        leadingTokens.emplace_back(
            std::nullopt,
            TokenKind::New(TokenKind::Identifier),
            t_compilation->Package.Name
        );
        leadingTokens.emplace_back(
            std::nullopt,
            TokenKind::New(TokenKind::Colon)
        );
        leadingTokens.emplace_back(
            std::nullopt,
            TokenKind::New(TokenKind::ModuleKeyword)
        );
        leadingTokens.emplace_back(
            std::nullopt,
            TokenKind::New(TokenKind::OpenBrace)
        );

        t_tokens.insert(
            begin(t_tokens),
            begin(leadingTokens),
            end  (leadingTokens)
        );
        t_tokens.insert(
            end(t_tokens) - 1,
            Token{ std::nullopt, TokenKind::New(TokenKind::CloseBrace) }
        );

        auto it = begin(t_tokens);

        ACE_TRY(module, ParseModule({ it, t_compilation->GlobalScope }));
        it += module.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::EndOfFile));
        ++it;

        ACE_TRY_ASSERT(it == end(t_tokens));

        return module.Value;
    }

    auto Parser::CreateEmptyAttributes() -> std::vector<std::shared_ptr<const Node::Attribute>>
    {
        return {};
    }

    // TODO: Turn this into a map maybe... std::unordered_map<TokenKind::Set, std::map<size_t, const char*>>;
    auto Parser::GetOperatorFunctionName(
        const Token& t_operatorToken,
        const size_t& t_parameterCount
    ) -> Expected<const char*>
    {
        const auto& tokenKind = t_operatorToken.Kind;
        const auto& stringValue = t_operatorToken.String;
        if (tokenKind == TokenKind::New(TokenKind::Asterisk))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::Multiplication;
        }

        if (tokenKind == TokenKind::New(TokenKind::Slash))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::Division;
        }

        if (tokenKind == TokenKind::New(TokenKind::Percent))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::Remainder;
        }

        if (tokenKind == TokenKind::New(TokenKind::Plus))
        {
            if (t_parameterCount == 1)
            {
                return SpecialIdentifier::Operator::UnaryPlus;
            }
            else if (t_parameterCount == 2)
            {
                return SpecialIdentifier::Operator::Addition;
            }

            ACE_TRY_UNREACHABLE();
        }

        if (tokenKind == TokenKind::New(TokenKind::Minus))
        {
            if (t_parameterCount == 1)
            {
                return SpecialIdentifier::Operator::UnaryNegation;
            }
            else if (t_parameterCount == 2)
            {
                return SpecialIdentifier::Operator::Subtraction;
            }

            ACE_TRY_UNREACHABLE();
        }

        if (tokenKind == TokenKind::New(TokenKind::LessThan))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::LessThan;
        }

        if (tokenKind == TokenKind::New(TokenKind::GreaterThan))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::GreaterThan;
        }

        if (tokenKind == TokenKind::New(TokenKind::LessThanEquals))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::LessThanEquals;
        }

        if (tokenKind == TokenKind::New(TokenKind::GreaterThanEquals))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::GreaterThanEquals;
        }

        if (tokenKind == TokenKind::New(TokenKind::GreaterThanGreaterThan))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::RightShift;
        }
        
        if (tokenKind == TokenKind::New(TokenKind::LessThanLessThan))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::LeftShift;
        }

        if (tokenKind == TokenKind::New(TokenKind::EqualsEquals))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::Equals;
        }

        if (tokenKind == TokenKind::New(TokenKind::ExclamationEquals))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::NotEquals;
        }

        if (tokenKind == TokenKind::New(TokenKind::Caret))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::XOR;
        }

        if (tokenKind == TokenKind::New(TokenKind::VerticalBar))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::OR;
        }

        if (tokenKind == TokenKind::New(TokenKind::Ampersand))
        {
            ACE_TRY_ASSERT(t_parameterCount == 2);
            return SpecialIdentifier::Operator::AND;
        }
        
        if (tokenKind == TokenKind::New(TokenKind::Tilde))
        {
            ACE_TRY_ASSERT(t_parameterCount == 1);
            return SpecialIdentifier::Operator::OneComplement;
        }
        
        if (tokenKind == TokenKind::New(TokenKind::ImplKeyword))
        {
            ACE_TRY_ASSERT(t_parameterCount == 1);
            return SpecialIdentifier::Operator::ImplicitFrom;
        }
        
        if (tokenKind == TokenKind::New(TokenKind::ExplKeyword))
        {
            ACE_TRY_ASSERT(t_parameterCount == 1);
            return SpecialIdentifier::Operator::ExplicitFrom;
        }

        if (tokenKind == TokenKind::New(TokenKind::Identifier))
        {
            if (stringValue == SpecialIdentifier::Copy)
            {
                ACE_TRY_ASSERT(t_parameterCount == 2);
                return SpecialIdentifier::Operator::Copy;
            }
            
            if (stringValue == SpecialIdentifier::Drop)
            {
                ACE_TRY_ASSERT(t_parameterCount == 1);
                return SpecialIdentifier::Operator::Drop;
            }

            ACE_TRY_UNREACHABLE();
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseName(Context t_context) -> Expected<ParseData<std::string>>
    {
        ACE_TRY_ASSERT(t_context.Iterator->Kind == TokenKind::New(TokenKind::Identifier));

        return ParseData
        {
            t_context.Iterator->String,
            1
        };
    }

    auto Parser::ParseNestedName(Context t_context) -> Expected<ParseData<std::vector<std::string>>>
    {
        std::vector<std::string> name{};
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Identifier));
        name.push_back(it->String);
        ++it;

        while (it->Kind == TokenKind::New(TokenKind::ColonColon))
        {
            ++it;

            ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Identifier));
            name.push_back(it->String);
            ++it;
        }

        return ParseData
        {
            std::move(name),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseSymbolName(Context t_context) -> Expected<ParseData<SymbolName>>
    {
        auto it = t_context.Iterator;

        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (it->Kind == TokenKind::New(TokenKind::ColonColon))
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            ++it;
        }
        
        std::vector<SymbolNameSection> sections{};

        ACE_TRY(section, ParseSymbolNameSection({ it, t_context.Scope }));
        sections.push_back(std::move(section.Value));
        it += section.Length;

        while (it->Kind == TokenKind::New(TokenKind::ColonColon))
        {
            ++it;

            ACE_TRY(section, ParseSymbolNameSection({ it, t_context.Scope }));
            sections.push_back(std::move(section.Value));
            it += section.Length;
        }

        return ParseData
        {
            SymbolName 
            { 
                sections,
                resolutionScope,
            },
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseSymbolNameSection(Context t_context) -> Expected<ParseData<SymbolNameSection>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Identifier));
        const std::string& name = it->String;
        ++it;

        auto templateArguments = [&]() -> std::vector<SymbolName>
        {
            auto expTemplateArguments = ParseTemplateArguments({ it, t_context.Scope });
            if (!expTemplateArguments)
                return {};

            it += expTemplateArguments.Unwrap().Length;
            return expTemplateArguments.Unwrap().Value;
        }();

        return ParseData
        {
           SymbolNameSection 
            {
                name,
                templateArguments
            },
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseTypeName(Context t_context, const bool& t_doAllowReferences) -> Expected<ParseData<TypeName>>
    {
        auto it = t_context.Iterator;

        std::vector<TypeNameModifier> modifiers{};

        if (t_doAllowReferences)
        {
            if (it->Kind == TokenKind::New(TokenKind::Ampersand))
            {
                modifiers.push_back(TypeNameModifier::Reference);
                ++it;
            }
        }

        for (; true; ++it)
        {
            if (it->Kind == TokenKind::New(TokenKind::Asterisk))
            {
                modifiers.push_back(TypeNameModifier::StrongPointer);
            }
            else if (it->Kind == TokenKind::New(TokenKind::Tilde))
            {
                modifiers.push_back(TypeNameModifier::WeakPointer);
            }
            else
            {
                break;
            }
        }

        ACE_TRY(symbolName, ParseSymbolName({ it, t_context.Scope }));
        it += symbolName.Length;

        return ParseData
        {
            TypeName
            {
                symbolName.Value,
                modifiers
            },
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseTemplateParameterNames(Context t_context) -> Expected<ParseData<std::vector<std::string>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBracket));
        ++it;

        std::vector<std::string> names{};
        while (it->Kind != TokenKind::New(TokenKind::CloseBracket))
        {
            if (names.size() != 0)
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Comma));
                ++it;
            }

            ACE_TRY(name, ParseName({ it, t_context.Scope }));
            names.push_back(std::move(name.Value));
            it += name.Length;
        }

        ACE_TRY_ASSERT(names.size() != 0);
        ++it;

        return ParseData
        {
            std::move(names),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseImplTemplateParameters(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>>>
    {
        ACE_TRY(names, ParseTemplateParameterNames(t_context));
        
        std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>> parameters{};
        std::transform(
            begin(names.Value),
            end  (names.Value),
            back_inserter(parameters),
            [&](const std::string& t_name)
            {
                return std::make_shared<const Node::TemplateParameter::Impl>(
                    t_context.Scope,
                    t_name
                );
            }
        );

        return ParseData
        {
             parameters,
             names.Length
        };
    }

    auto Parser::ParseTemplateParameters(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>>>>
    {
        ACE_TRY(names, ParseTemplateParameterNames(t_context));
        
        std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>> parameters{};
        std::transform(
            begin(names.Value),
            end  (names.Value),
            back_inserter(parameters),
            [&](const std::string& t_name)
            {
                return std::make_shared<const Node::TemplateParameter::Normal>(
                    t_context.Scope,
                    t_name
                );
            }
        );

        return ParseData
        {
             parameters,
             names.Length
        };
    }

    auto Parser::ParseTemplateArguments(Context t_context) -> Expected<ParseData<std::vector<SymbolName>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBracket));
        ++it;

        std::vector<SymbolName> arguments{};
        while (it->Kind != TokenKind::New(TokenKind::CloseBracket))
        {
            if (arguments.size() != 0)
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Comma));
                ++it;
            }

            ACE_TRY(argument, ParseSymbolName({ it, t_context.Scope }));
            arguments.push_back(std::move(argument.Value));
            it += argument.Length;
        }

        ACE_TRY_ASSERT(arguments.size() != 0);
        ++it;

        return ParseData
        {
            std::move(arguments),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseModule(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Module>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(name, ParseNestedName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::ModuleKeyword));
        ++it;

        auto accessModifier = AccessModifier::Private;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        std::vector<std::shared_ptr<Scope>> scopes{};
        scopes.push_back(t_context.Scope);
        std::transform(
            begin(name.Value),
            end  (name.Value),
            back_inserter(scopes),
            [&](const std::string& t_name)
            {
                return scopes.back()->GetOrCreateChild(t_name);
            }
        );

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBrace));
        ++it;

        std::vector<std::shared_ptr<const Node::Module>> modules{};
        std::vector<std::shared_ptr<const Node::Type::IBase>> types{};
        std::vector<std::shared_ptr<const Node::Template::Type>> typeTemplates{};
        std::vector<std::shared_ptr<const Node::Impl>> impls{};
        std::vector<std::shared_ptr<const Node::TemplatedImpl>> templatedImpls{};
        std::vector<std::shared_ptr<const Node::Function>> functions{};
        std::vector<std::shared_ptr<const Node::Template::Function>> functionTemplates{};
        std::vector<std::shared_ptr<const Node::Variable::Normal::Static>> variables{};

        while (it->Kind != TokenKind::New(TokenKind::CloseBrace))
        {
            const auto selfScope = scopes.back();

            if (const auto expModule = Parser::ParseModule({ it, selfScope }))
            {
                modules.push_back(expModule.Unwrap().Value);
                it += expModule.Unwrap().Length;
                continue;
            }

            if (const auto expType = ParseType({ it, selfScope }))
            {
                types.push_back(expType.Unwrap().Value);
                it += expType.Unwrap().Length;
                continue;
            }

            if (const auto expTypeTemplate = ParseTypeTemplate({ it, selfScope }))
            {
                typeTemplates.push_back(expTypeTemplate.Unwrap().Value);
                it += expTypeTemplate.Unwrap().Length;
                continue;
            }

            if (const auto expImpl = ParseImpl({ it, selfScope }))
            {
                impls.push_back(expImpl.Unwrap().Value);
                it += expImpl.Unwrap().Length;
                continue;
            }

            if (const auto expTemplatedImpl = ParseTemplatedImpl({ it, selfScope }))
            {
                templatedImpls.push_back(expTemplatedImpl.Unwrap().Value);
                it += expTemplatedImpl.Unwrap().Length;
                continue;
            }

            if (const auto expFunction = ParseFunction({ it, selfScope }))
            {
                functions.push_back(expFunction.Unwrap().Value);
                it += expFunction.Unwrap().Length;
                continue;
            }

            if (const auto expFunctionTemplate = ParseFunctionTemplate({ it, selfScope }))
            {
                functionTemplates.push_back(expFunctionTemplate.Unwrap().Value);
                it += expFunctionTemplate.Unwrap().Length;
                continue;
            }

            if (const auto expVariable = ParseVariable({ it, selfScope }))
            {
                variables.push_back(expVariable.Unwrap().Value);
                it += expVariable.Unwrap().Length;
                continue;
            }

            ACE_TRY_UNREACHABLE();
        }

        ++it;

        return ParseData
        {
            std::make_shared<const Node::Module>(
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
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseImpl(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Impl>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::ImplKeyword));
        ++it;

        ACE_TRY(typeName, ParseSymbolName({ it, t_context.Scope }));

        // TODO: Remove this block after impl template specialization.
        {
            const bool foundTemplatedSection = std::find_if(begin(typeName.Value.Sections), end(typeName.Value.Sections),
            [](const SymbolNameSection& t_section)
            {
                return t_section.TemplateArguments.empty();
            }) == end(typeName.Value.Sections);

            ACE_TRY_ASSERT(!foundTemplatedSection);
        }

        it += typeName.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBrace));
        ++it;

        std::vector<std::shared_ptr<const Node::Function>> functions{};
        std::vector<std::shared_ptr<const Node::Template::Function>> functionTemplates{};

        while (it->Kind != TokenKind::New(TokenKind::CloseBrace))
        {
            if (const auto expFunction = ParseImplFunction({ it, scope }, typeName.Value))
            {
                functions.push_back(expFunction.Unwrap().Value);
                it += expFunction.Unwrap().Length;
                continue;
            }

            if (const auto expFunctionTemplate = ParseImplFunctionTemplate({ it, scope }, typeName.Value))
            {
                functionTemplates.push_back(expFunctionTemplate.Unwrap().Value);
                it += expFunctionTemplate.Unwrap().Length;
                continue;
            }

            ACE_TRY_UNREACHABLE();
        }

        ++it;

        return ParseData
        {
            std::make_shared<const Node::Impl>(
                scope,
                typeName.Value,
                functions,
                functionTemplates
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseImplFunction(Context t_context, const SymbolName& t_selfTypeName) -> Expected<ParseData<std::shared_ptr<const Node::Function>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(nameToken, [&]() -> Expected<Token>
        {
            if (it->Kind == TokenKind::New(TokenKind::OperatorKeyword)) 
            {
                ++it;

                const Token& operatorToken = *it;
                const bool isSpecialOperatorName = (operatorToken.Kind & OperatorTokenMask::User).none();
                if (isSpecialOperatorName)
                {
                    ACE_TRY_ASSERT(
                        operatorToken.Kind == TokenKind::New(TokenKind::ImplKeyword) ||
                        operatorToken.Kind == TokenKind::New(TokenKind::ExplKeyword) ||
                        operatorToken.Kind == TokenKind::New(TokenKind::Identifier) 
                    );
                }

                ++it;

                return operatorToken;
            }
            else
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Identifier));
                Token nameToken = *it;
                ++it;

                return nameToken;
            }
        }());

        ACE_TRY(parameters, ParseParameters({ it, scope }));
        it += parameters.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, false));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasSelfModifier = false;
        bool hasExternModifier = false;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Kind == TokenKind::New(TokenKind::Identifier))
            {
                ACE_TRY_ASSERT(it->String == SpecialIdentifier::Self);
                hasSelfModifier = true;
                ++it;
            }

            if (it->Kind == TokenKind::New(TokenKind::ExternKeyword))
            {
                hasExternModifier = true;
                ACE_TRY_ASSERT(!hasSelfModifier);
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(name, [&]() -> Expected<std::string>
        {
            if (nameToken.Kind == TokenKind::New(TokenKind::Identifier))
            {
                return nameToken.String;
            }
            else
            {
                ACE_TRY(name, GetOperatorFunctionName(
                    nameToken,
                    parameters.Value.size()
                ));

                ACE_TRY_ASSERT(accessModifier == AccessModifier::Public);
                ACE_TRY_ASSERT(!hasSelfModifier);

                return std::string{ name };
            }
        }());

        ACE_TRY(optBody, [&]() -> Expected<std::optional<std::shared_ptr<const Node::Statement::Block>>>
        {
            if (hasExternModifier)
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
                ++it;

                return std::optional<std::shared_ptr<const Node::Statement::Block>>{};
            }
            else
            {
                ACE_TRY(body, ParseBlockStatement({ it, scope }));
                it += body.Length;

                return std::optional{ body.Value };
            }
        }());

        const auto selfParameter = [&]() -> std::optional<std::shared_ptr<const Node::Variable::Parameter::Self>>
        {
            if (!hasSelfModifier)
            {
                return std::nullopt;
            }

            return std::make_shared<const Node::Variable::Parameter::Self>(
                scope,
                t_selfTypeName
                );
        }();

        return ParseData
        {
            std::make_shared<const Node::Function>(
                scope,
                name,
                typeName.Value,
                attributes.Value,
                accessModifier,
                selfParameter,
                parameters.Value,
                optBody
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseImplFunctionTemplate(Context t_context, const SymbolName& t_selfTypeName) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(templateParameters, ParseTemplateParameters({ it, scope }));
        it += templateParameters.Length;

        ACE_TRY(parameters, ParseParameters({ it, scope }));
        it += parameters.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, false));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasSelfModifier = false;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Kind == TokenKind::New(TokenKind::Identifier))
            {
                ACE_TRY_ASSERT(it->String == SpecialIdentifier::Self);
                hasSelfModifier = true;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseBlockStatement({ it, scope }));
        it += body.Length;

        const auto selfParameter = [&]() -> std::optional<std::shared_ptr<const Node::Variable::Parameter::Self>>
        {
            if (!hasSelfModifier)
            {
                return std::nullopt;
            }

            return std::make_shared<const Node::Variable::Parameter::Self>(
                scope,
                t_selfTypeName
                );
        }();

        const auto function = std::make_shared<const Node::Function>(
            scope,
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            selfParameter,
            parameters.Value,
            body.Value
            );

        return ParseData
        {
            std::make_shared<const Node::Template::Function>(
                std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>{},
                templateParameters.Value,
                function
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseTemplatedImpl(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::TemplatedImpl>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::ImplKeyword));
        ++it;

        ACE_TRY(templateParameters, ParseImplTemplateParameters({ it, scope }));
        it += templateParameters.Length;

        ACE_TRY(typeName, ParseSymbolName({ it, t_context.Scope }));

        // TODO: Remove this block after impl template specialization.
        {
            const bool foundTemplatedSection = std::find_if(begin(typeName.Value.Sections), end(typeName.Value.Sections) - 1,
            [](const SymbolNameSection& t_section)
            {
                return t_section.TemplateArguments.empty();
            }) == end(typeName.Value.Sections);

            ACE_TRY_ASSERT(!foundTemplatedSection);

            const auto& templateArguments = typeName.Value.Sections.back().TemplateArguments;
            ACE_TRY_ASSERT(templateParameters.Value.size() == templateArguments.size());

            std::unordered_set<std::string> templateParameterSet{};
            ACE_TRY_VOID(TransformExpectedVector(templateParameters.Value,
            [&](const std::shared_ptr<const Node::TemplateParameter::Impl>& t_templateParameter) -> Expected<void>
            {
                const std::string& templateParameterName = t_templateParameter->GetName();
                ACE_TRY_ASSERT(!templateParameterSet.contains(templateParameterName));
                templateParameterSet.insert(templateParameterName);

                return ExpectedVoid;
            }));
            
            ACE_TRY_VOID(TransformExpectedVector(templateArguments,
            [&](const SymbolName& t_argument) -> Expected<void>
            {
                ACE_TRY_ASSERT(t_argument.Sections.size() == 1);
                ACE_TRY_ASSERT(t_argument.Sections.back().TemplateArguments.empty());

                const std::string& templateArgumentName = t_argument.Sections.front().Name;
                ACE_TRY_ASSERT(templateParameterSet.contains(templateArgumentName));
                templateParameterSet.erase(templateArgumentName);

                return ExpectedVoid;
            }));
        }

        it += typeName.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBrace));
        ++it;

        std::vector<std::shared_ptr<const Node::Template::Function>> functionTemplates{};

        while (it->Kind != TokenKind::New(TokenKind::CloseBrace))
        {
            auto expFunctionTemplate = ParseTemplatedImplFunction(
                { it, scope },
                typeName.Value,
                templateParameters.Value
            );
            if (expFunctionTemplate)
            {
                functionTemplates.push_back(expFunctionTemplate.Unwrap().Value);
                it += expFunctionTemplate.Unwrap().Length;
                continue;
            }

            ACE_TRY_UNREACHABLE();
        }

        ++it;

        auto typeTemplateName = typeName.Value;
        auto& typeTemplateNameLastSection = typeTemplateName.Sections.back();
        typeTemplateNameLastSection.TemplateArguments.clear();
        typeTemplateNameLastSection.Name = SpecialIdentifier::CreateTemplate(typeTemplateNameLastSection.Name);

        return ParseData
        {
            std::make_shared<const Node::TemplatedImpl>(
                scope,
                typeTemplateName,
                std::vector<std::shared_ptr<const Node::Function>>{},
                functionTemplates
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseTemplatedImplFunction(Context t_context, const SymbolName& t_selfTypeName, const std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>& t_implTemplateParameters) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(nameToken, [&]() -> Expected<Token>
        {
            if (it->Kind == TokenKind::New(TokenKind::OperatorKeyword))
            {
                ++it;

                const Token& operatorToken = *it;
                const bool isSpecialOperatorName = (operatorToken.Kind & OperatorTokenMask::User).none();
                if (isSpecialOperatorName)
                {
                    ACE_TRY_ASSERT(
                        operatorToken.Kind == TokenKind::New(TokenKind::ImplKeyword) ||
                        operatorToken.Kind == TokenKind::New(TokenKind::ExplKeyword) ||
                        operatorToken.Kind == TokenKind::New(TokenKind::Identifier)
                    );
                }

                ++it;

                return operatorToken;
            }
            else
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Identifier));
                Token nameToken = *it;
                ++it;

                return nameToken;
            }
        }());

        ACE_TRY(templateParameters, [&]() -> Expected<std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>>>
        {
            if (const auto expTemplateParameters = ParseTemplateParameters({ it, scope }))
            {
                it += expTemplateParameters.Unwrap().Length;
                return expTemplateParameters.Unwrap().Value;
            }
            else
            {
                return std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>>{};
            }
        }());

        ACE_TRY(parameters, ParseParameters({ it, scope }));
        it += parameters.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, false));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasSelfModifier = false;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Kind == TokenKind::New(TokenKind::Identifier))
            {
                ACE_TRY_ASSERT(it->String == SpecialIdentifier::Self);
                hasSelfModifier = true;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(name, [&]() -> Expected<std::string>
        {
            if (nameToken.Kind == TokenKind::New(TokenKind::Identifier))
            {
                return nameToken.String;
            }
            else
            {
                ACE_TRY(name, GetOperatorFunctionName(
                    nameToken,
                    parameters.Value.size()
                ));

                ACE_TRY_ASSERT(accessModifier == AccessModifier::Public);
                ACE_TRY_ASSERT(!hasSelfModifier);

                return std::string{ name };
            }
        }());

        ACE_TRY(body, ParseBlockStatement({ it, scope }));
        it += body.Length;

        const auto selfParameter = [&]() -> std::optional<std::shared_ptr<const Node::Variable::Parameter::Self>>
        {
            if (!hasSelfModifier)
            {
                return std::nullopt;
            }

            return std::make_shared<const Node::Variable::Parameter::Self>(
                scope,
                t_selfTypeName
                );
        }();

        const auto function = std::make_shared<const Node::Function>(
            scope,
            name,
            typeName.Value,
            attributes.Value,
            accessModifier,
            selfParameter,
            parameters.Value,
            body.Value
            );

        std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>> clonedImplTemplateParameters{};
        std::transform(
            begin(t_implTemplateParameters),
            end  (t_implTemplateParameters),
            back_inserter(clonedImplTemplateParameters),
            [&](const std::shared_ptr<const Node::TemplateParameter::Impl>& t_implTemplateParameter)
            {
                return t_implTemplateParameter->CloneInScope(scope);
            }
        );


        return ParseData
        {
            std::make_shared<const Node::Template::Function>(
                clonedImplTemplateParameters,
                templateParameters,
                function
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseFunction(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Function>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(parameters, ParseParameters({ it, scope }));
        it += parameters.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, false));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasExternModifier = false;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Kind == TokenKind::New(TokenKind::ExternKeyword))
            {
                hasExternModifier = true;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(optBody, [&]() -> Expected<std::optional<std::shared_ptr<const Node::Statement::Block>>>
        {
            if (hasExternModifier)
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
                ++it;

                return std::optional<std::shared_ptr<const Node::Statement::Block>>{};
            }
            else
            {
                ACE_TRY(body, ParseBlockStatement({ it, scope }));
                it += body.Length;

                return std::optional{ body.Value };
            }
        }());

        return ParseData
        {
            std::make_shared<const Node::Function>(
                scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                accessModifier,
                std::nullopt,
                parameters.Value,
                optBody
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseFunctionTemplate(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Function>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(templateParameters, [&]() -> Expected<std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>>>
        {
            if (const auto expTemplateParameters = ParseTemplateParameters({ it, scope }))
            {
                it += expTemplateParameters.Unwrap().Length;
                return expTemplateParameters.Unwrap().Value;
            }
            else
            {
                return std::vector<std::shared_ptr<const Node::TemplateParameter::Normal>>{};
            }
        }());

        ACE_TRY(parameters, ParseParameters({ it, scope }));
        it += parameters.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, false));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseBlockStatement({ it, scope }));
        it += body.Length;

        const auto function = std::make_shared<const Node::Function>(
            scope,
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            std::nullopt,
            parameters.Value,
            body.Value
            );

        return ParseData
        {
            std::make_shared<const Node::Template::Function>(
                std::vector<std::shared_ptr<const Node::TemplateParameter::Impl>>{},
                templateParameters,
                function
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseParameters(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>>>>
    {
        std::vector<std::shared_ptr<const Node::Variable::Parameter::Normal>> parameters{};
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenParen));
        ++it;

        bool isFirstParameter = true;
        while (it->Kind != TokenKind::New(TokenKind::CloseParen))
        {
            if (isFirstParameter)
            {
                isFirstParameter = false;
            }
            else
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Comma));
                ++it;
            }

            ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
            it += attributes.Length;

            ACE_TRY(name, ParseName({ it, t_context.Scope }));
            it += name.Length;

            ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
            ++it;

            ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, true));
            it += typeName.Length;

            parameters.push_back(std::make_shared<const Node::Variable::Parameter::Normal>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                parameters.size()
                ));
        }

        ++it;

        return ParseData
        {
            parameters,
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseVariable(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Variable::Normal::Static>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, false));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Variable::Normal::Static>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                accessModifier
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseMemberVariable(Context t_context, const size_t& t_index) -> Expected<ParseData<std::shared_ptr<const Node::Variable::Normal::Instance>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, false));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        return ParseData
        {
            std::make_shared<const Node::Variable::Normal::Instance>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                accessModifier,
                t_index
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseType(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Type::IBase>>>
    {
        if (const auto expStruct = ParseStruct({ t_context.Iterator, t_context.Scope }))
        {
            return ParseData<std::shared_ptr<const Node::Type::IBase>>
            {
                expStruct.Unwrap().Value,
                expStruct.Unwrap().Length
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseTypeTemplate(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Type>>>
    {
        if (const auto expStructTemplate = ParseStructTemplate({ t_context.Iterator, t_context.Scope }))
        {
            return expStructTemplate;
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseStruct(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Type::Struct>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::StructKeyword));
        ++it;

        auto accessModifier = AccessModifier::Private;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseStructBody({ it, scope }));
        it += body.Length;

        return ParseData
        {
            std::make_shared<const Node::Type::Struct>(
                scope,
                name.Value,
                attributes.Value,
                accessModifier,
                body.Value
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseStructBody(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBrace));
        ++it;

        std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>> variables{};
        while (it->Kind != TokenKind::New(TokenKind::CloseBrace))
        {
            if (variables.size() != 0)
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Comma));
                ++it;

                if (it->Kind == TokenKind::New(TokenKind::CloseBrace))
                    break;
            }

            ACE_TRY(variable, ParseMemberVariable({ it, t_context.Scope }, variables.size()));
            variables.push_back(variable.Value);
            it += variable.Length;
        }

        ++it;

        return ParseData<std::vector<std::shared_ptr<const Node::Variable::Normal::Instance>>>
        {
            variables,
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseStructTemplate(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Template::Type>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(templateParameters, ParseTemplateParameters({ it, scope }));
        it += templateParameters.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::StructKeyword));
        ++it;

        auto accessModifier = AccessModifier::Private;

        if (it->Kind == TokenKind::New(TokenKind::MinusGreaterThan))
        {
            ++it;
            const auto startIt = it;

            if (it->Kind == TokenKind::New(TokenKind::PublicKeyword))
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseStructBody({ it, scope }));
        it += body.Length;

        const auto type = std::make_shared<const Node::Type::Struct>(
            scope,
            name.Value,
            attributes.Value,
            accessModifier,
            body.Value
        );

        return ParseData
        {
            std::make_shared<const Node::Template::Type>(
                templateParameters.Value,
                type
            ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::IBase>>>
    {
        if (const auto expExpressionStatemment = ParseExpressionStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expExpressionStatemment.Unwrap().Value,
                expExpressionStatemment.Unwrap().Length
            };
        }

        if (const auto expAssignmentStatement = ParseAssignmentStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expAssignmentStatement.Unwrap().Value,
                expAssignmentStatement.Unwrap().Length
            };
        }

        if (const auto expCompoundAssignmentStatement = ParseCompoundAssignmentStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expCompoundAssignmentStatement.Unwrap().Value,
                expCompoundAssignmentStatement.Unwrap().Length
            };
        }

        if (const auto expVariableStatement = ParseVariableStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expVariableStatement.Unwrap().Value,
                expVariableStatement.Unwrap().Length
            };
        }

        if (const auto expKeywordStatement = ParseKeywordStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expKeywordStatement.Unwrap().Value,
                expKeywordStatement.Unwrap().Length
            };
        }

        if (const auto expCompoundStatement = ParseBlockStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expCompoundStatement.Unwrap().Value,
                expCompoundStatement.Unwrap().Length
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseExpressionStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Expression>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(expression, ParseExpression({ it, t_context.Scope }));
        it += expression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Statement::Expression>(expression.Value),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseAssignmentStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assignment::Normal>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(lhsExpression, ParseExpression({ it, t_context.Scope }));
        it += lhsExpression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Equals));
        ++it;

        ACE_TRY(rhsExpression, ParseExpression({ it, t_context.Scope }));
        it += rhsExpression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Statement::Assignment::Normal>(
                t_context.Scope,
                lhsExpression.Value,
                rhsExpression.Value
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseCompoundAssignmentStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assignment::Compound>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(lhsExpression, ParseExpression({ it, t_context.Scope }));
        it += lhsExpression.Length;

        ACE_TRY_ASSERT((it->Kind & OperatorTokenMask::CompoundAssignment).any());
        const auto op = it->Kind;
        ++it;

        ACE_TRY(rhsExpression, ParseExpression({ it, t_context.Scope }));
        it += rhsExpression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Statement::Assignment::Compound>(
                t_context.Scope,
                lhsExpression.Value,
                rhsExpression.Value,
                op
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseVariableStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Variable>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(name, ParseName(t_context));
        it += name.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Colon));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, true));
        it += typeName.Length;

        ACE_TRY(optAssignment, [&]() -> Expected<std::optional<std::shared_ptr<const Node::Expression::IBase>>>
        {
            if (it->Kind == TokenKind::New(TokenKind::Semicolon))
            {
                ++it;
                return std::optional<std::shared_ptr<const Node::Expression::IBase>>{};
            }
            else if (it->Kind == TokenKind::New(TokenKind::Equals))
            {
                ++it;

                ACE_TRY(expression, ParseExpression({ it, t_context.Scope }));
                it += expression.Length;

                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
                ++it;

                return std::optional{ expression.Value };
            }

            ACE_TRY_UNREACHABLE();
        }());

        return ParseData
        {
            std::make_shared<const Node::Statement::Variable>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                optAssignment
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseKeywordStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::IBase>>>
    {
        if (const auto expIfStatement = ParseIfStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expIfStatement.Unwrap().Value,
                expIfStatement.Unwrap().Length
            };
        }

        if (const auto expWhileStatement = ParseWhileStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expWhileStatement.Unwrap().Value,
                expWhileStatement.Unwrap().Length
            };
        }

        if (const auto expReturnStatement = ParseReturnStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expReturnStatement.Unwrap().Value,
                expReturnStatement.Unwrap().Length
            };
        }

        if (const auto expExitStatement = ParseExitStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expExitStatement.Unwrap().Value,
                expExitStatement.Unwrap().Length
            };
        }

        if (const auto expAssertStatement = ParseAssertStatement(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Statement::IBase>>
            {
                expAssertStatement.Unwrap().Value,
                expAssertStatement.Unwrap().Length
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseIfStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::If>>>
    {
        auto it = t_context.Iterator;

        std::vector<std::shared_ptr<const Node::Expression::IBase>> conditions{};
        std::vector<std::shared_ptr<const Node::Statement::Block>> bodies{};

        ACE_TRY(ifBlock, ParseIfBlock({ it, t_context.Scope }));
        it += ifBlock.Length;

        conditions.push_back(ifBlock.Value.first);
        bodies.push_back(ifBlock.Value.second);

        while (it->Kind == TokenKind::New(TokenKind::ElifKeyword))
        {
            ACE_TRY(elifBlock, ParseElifBlock({ it, t_context.Scope }));

            conditions.push_back(elifBlock.Value.first);
            bodies.push_back(elifBlock.Value.second);
            it += elifBlock.Length;
        }

        if (const auto expElseBlock = ParseElseBlock({ it, t_context.Scope }))
        {
            bodies.push_back(expElseBlock.Unwrap().Value);
            it += expElseBlock.Unwrap().Length;
        }

        return ParseData
        {
            std::make_shared<const Node::Statement::If>(
                t_context.Scope,
                conditions,
                bodies
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseIfBlock(Context t_context) -> Expected<ParseData<std::pair<std::shared_ptr<const Node::Expression::IBase>, std::shared_ptr<const Node::Statement::Block>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::IfKeyword));
        ++it;

        ACE_TRY(condition, ParseExpression({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY(body, ParseBlockStatement({ it, t_context.Scope }));
        it += body.Length;

        return ParseData
        {
            std::pair
            {
                condition.Value,
                body.Value
            },
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseElifBlock(Context t_context) -> Expected<ParseData<std::pair<std::shared_ptr<const Node::Expression::IBase>, std::shared_ptr<const Node::Statement::Block>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::ElifKeyword));
        ++it;

        ACE_TRY(condition, ParseExpression({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY(body, ParseBlockStatement({ it, t_context.Scope }));
        it += body.Length;

        return ParseData
        {
            std::pair
            {
                condition.Value,
                body.Value
            },
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseElseBlock(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Block>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::ElseKeyword));
        ++it;

        ACE_TRY(body, ParseBlockStatement({ it, t_context.Scope }));
        it += body.Length;

        return ParseData
        {
            body.Value,
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseWhileStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::While>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::WhileKeyword));
        ++it;

        ACE_TRY(condition, ParseExpression({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY(body, ParseBlockStatement({ it, t_context.Scope }));
        it += body.Length;

        return ParseData
        {
            std::make_shared<const Node::Statement::While>(
                t_context.Scope,
                condition.Value,
                body.Value
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseReturnStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Return>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::ReturnKeyword));
        ++it;

        ACE_TRY(optExpression, [&]() -> Expected<std::optional<std::shared_ptr<const Node::Expression::IBase>>>
        {
            if (it->Kind == TokenKind::New(TokenKind::Semicolon))
            {
                ++it;
                return std::optional<std::shared_ptr<const Node::Expression::IBase>>{};
            }

            ACE_TRY(expression, ParseExpression({ it, t_context.Scope }));
            it += expression.Length;

            ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
            ++it;

            return std::optional{ expression.Value };
        }());

        return ParseData
        {
            std::make_shared<const Node::Statement::Return>(
                t_context.Scope,
                optExpression
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseExitStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Exit>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::ExitKeyword));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Statement::Exit>(t_context.Scope),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseAssertStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Assert>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::AssertKeyword));
        ++it;

        ACE_TRY(condition, ParseExpression({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Semicolon));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Statement::Assert>(
                t_context.Scope,
                condition.Value
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseBlockStatement(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Statement::Block>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBrace));
        ++it;

        std::vector<std::shared_ptr<const Node::Statement::IBase>> statements{};
        while (it->Kind != TokenKind::New(TokenKind::CloseBrace))
        {
            ACE_TRY(statement, ParseStatement({ it, scope }));
            statements.push_back(statement.Value);
            it += statement.Length;
        }

        ++it;

        return ParseData
        {
            std::make_shared<const Node::Statement::Block>(
                scope,
                statements
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>
    {
        static constexpr size_t binaryOperatorPrecedenceMin = 9;
        static const auto getBinaryOperatorPrecedence = [](const TokenKind::Set& t_tokenKind) -> size_t
        {
            if (
                (t_tokenKind == TokenKind::New(TokenKind::Asterisk)) ||
                (t_tokenKind == TokenKind::New(TokenKind::Slash)) ||
                (t_tokenKind == TokenKind::New(TokenKind::Percent))
                )
            {
                return 0;
            }

            if (
                (t_tokenKind == TokenKind::New(TokenKind::Plus)) ||
                (t_tokenKind == TokenKind::New(TokenKind::Minus))
                )
            {
                return 1;
            }

            if (
                (t_tokenKind == TokenKind::New(TokenKind::LessThanLessThan)) ||
                (t_tokenKind == TokenKind::New(TokenKind::GreaterThanGreaterThan))
                )
            {
                return 2;
            }

            if (
                (t_tokenKind == TokenKind::New(TokenKind::LessThan)) ||
                (t_tokenKind == TokenKind::New(TokenKind::LessThanEquals)) ||
                (t_tokenKind == TokenKind::New(TokenKind::GreaterThan)) ||
                (t_tokenKind == TokenKind::New(TokenKind::GreaterThanEquals))
                )
            {
                return 3;
            }

            if (
                (t_tokenKind == TokenKind::New(TokenKind::EqualsEquals)) ||
                (t_tokenKind == TokenKind::New(TokenKind::ExclamationEquals))
                )
            {
                return 4;
            }

            if (t_tokenKind == TokenKind::New(TokenKind::Ampersand))
            {
                return 5;
            }

            if (t_tokenKind == TokenKind::New(TokenKind::Caret))
            {
                return 6;
            }

            if (t_tokenKind == TokenKind::New(TokenKind::VerticalBar))
            {
                return 7;
            }

            if (t_tokenKind == TokenKind::New(TokenKind::AmpersandAmpersand))
            {
                return 8;
            }

            if (t_tokenKind == TokenKind::New(TokenKind::VerticalBarVerticalBar))
            {
                return binaryOperatorPrecedenceMin;
            }

            ACE_UNREACHABLE();
        };
        static const auto getBinaryOperatorPrecedencesAssociativity = [](const size_t& t_precedence) -> OperatorAssociativity
        {
            switch (t_precedence)
            {
                case 0: return OperatorAssociativity::LeftToRight;
                case 1: return OperatorAssociativity::LeftToRight;
                case 2: return OperatorAssociativity::LeftToRight;
                case 3: return OperatorAssociativity::LeftToRight;
                case 4: return OperatorAssociativity::LeftToRight;
                case 5: return OperatorAssociativity::LeftToRight;
                case 6: return OperatorAssociativity::LeftToRight;
                case 7: return OperatorAssociativity::LeftToRight;
                case 8: return OperatorAssociativity::LeftToRight;
                case 9: return OperatorAssociativity::LeftToRight;
            }

            ACE_UNREACHABLE();
        };

        auto it = t_context.Iterator;

        ACE_TRY(simpleExpression, ParseSimpleExpression({ it, t_context.Scope }));
        it += simpleExpression.Length;

        std::vector<std::shared_ptr<const Node::Expression::IBase>> expressions{};
        std::vector<std::reference_wrapper<const TokenKind::Set>> operators{};

        expressions.push_back(simpleExpression.Value);
        
        while ((it->Kind & OperatorTokenMask::Binary::All).any())
        {
            operators.push_back(it->Kind);
            ++it;

            ACE_TRY(simpleExpression, ParseSimpleExpression({ it, t_context.Scope }));
            expressions.push_back(simpleExpression.Value);
            it += simpleExpression.Length;
        }

        const auto collapseOperator = [&](const size_t& t_index)
        {
            const auto& lhsExpression = expressions.at(t_index);
            const auto& rhsExpression = expressions.at(t_index + 1);

            const auto collapsedExpression = [&]() -> std::shared_ptr<const Node::Expression::IBase>
            {
                const auto& tokenKind = operators.at(t_index).get();

                if (tokenKind == TokenKind::New(TokenKind::AmpersandAmpersand))
                {
                    return std::make_shared<const Node::Expression::And>(
                        lhsExpression,
                        rhsExpression
                    );
                }
                else if (tokenKind == TokenKind::New(TokenKind::AmpersandAmpersand))
                {
                    return std::make_shared<const Node::Expression::And>(
                        lhsExpression,
                        rhsExpression
                    );
                }
                else if (tokenKind == TokenKind::New(TokenKind::VerticalBarVerticalBar))
                {
                    return std::make_shared<const Node::Expression::Or>(
                        lhsExpression,
                        rhsExpression
                    );
                }
                else
                {
                    return std::make_shared<const Node::Expression::BinaryUser>(
                        lhsExpression,
                        rhsExpression,
                        tokenKind
                    );
                }
            }();

            operators.erase(begin(operators) + t_index);
            expressions.erase(begin(expressions) + t_index + 1);
            expressions.at(t_index) = collapsedExpression;
        };
        
        for (size_t precendence = 0; precendence <= binaryOperatorPrecedenceMin; precendence++)
        {
            const auto associativity = getBinaryOperatorPrecedencesAssociativity(precendence);

            bool didCollapseAll = false;
            while (!didCollapseAll)
            {
                if (operators.empty())
                    break;

                const auto [startOperatorIndex, endOperatorIndex, operatorIndexIncrement] = [&]() -> std::tuple<int, int, int>
                {
                    switch (associativity)
                    {
                        case OperatorAssociativity::LeftToRight: return { 0, static_cast<int>(operators.size()), 1 };
                        case OperatorAssociativity::RightToLeft: return { static_cast<int>(operators.size()) - 1, -1, -1 };

                        default: ACE_UNREACHABLE();
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

        ACE_TRY_ASSERT((expressions.size() == 1) && (operators.empty()));

        return ParseData
        {
            expressions.front(),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseSimpleExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>
    {
        static constexpr size_t unaryOperatorPrecedenceMin = 1;
        static const auto getUnaryOperatorPrecedence = [](const TokenKind::Set& t_tokenKind) -> size_t
        {
            if (t_tokenKind == TokenKind::New(TokenKind::OpenParen))
            {
                return 0;
            }

            if (
                (t_tokenKind == TokenKind::New(TokenKind::Plus)) ||
                (t_tokenKind == TokenKind::New(TokenKind::Minus)) ||
                (t_tokenKind == TokenKind::New(TokenKind::Tilde)) ||
                (t_tokenKind == TokenKind::New(TokenKind::Exclamation)) ||
                (t_tokenKind == TokenKind::New(TokenKind::BoxKeyword)) ||
                (t_tokenKind == TokenKind::New(TokenKind::UnboxKeyword))
                )
            {
                return unaryOperatorPrecedenceMin;
            }

            ACE_UNREACHABLE();
        };

        auto it = t_context.Iterator;

        std::vector<std::reference_wrapper<const TokenKind::Set>> prefixOperators{};
        std::vector<std::pair<std::reference_wrapper<const TokenKind::Set>, std::vector<std::shared_ptr<const Node::Expression::IBase>>>> postfixOperators{};

        while ((it->Kind & OperatorTokenMask::Prefix::All).any())
        {
            const auto& tokenKind = it->Kind;
            ++it;

            prefixOperators.emplace_back(tokenKind);
        }

        ACE_TRY(primaryExpression, ParsePrimaryExpression({ it, t_context.Scope }));

        ACE_TRY(rootExpression, [&]() -> Expected<std::shared_ptr<const Node::Expression::IBase>>
        {
            if (const auto expMemberAccessExpression = ParseMemberAccessExpression({ it, t_context.Scope }))
            {
                it += expMemberAccessExpression.Unwrap().Length;
                return std::shared_ptr<const Node::Expression::IBase>
                {
                    expMemberAccessExpression.Unwrap().Value
                };
            }

            it += primaryExpression.Length;
            return primaryExpression.Value;
        }());

        while ((it->Kind & OperatorTokenMask::Postfix).any())
        {
            const auto& tokenKind = it->Kind;

            ACE_TRY(arguments, [&]() -> Expected<std::vector<std::shared_ptr<const Node::Expression::IBase>>>
            {
                if (it->Kind != TokenKind::New(TokenKind::OpenParen))
                {
                    ++it;
                    return std::vector<std::shared_ptr<const Node::Expression::IBase>>{};
                }

                ACE_TRY(arguments, ParseArguments({ it, t_context.Scope }));
                it += arguments.Length;
                return arguments.Value;
            }());

            postfixOperators.emplace_back(tokenKind, arguments);
        }

        auto expression = rootExpression;

        const auto collapseLHS = [&]()
        {
            const auto& tokenKind = prefixOperators.front();

            const auto collapsedExpression = [&]() -> std::shared_ptr<const Node::Expression::IBase>
            {
                if (tokenKind == TokenKind::New(TokenKind::Exclamation))
                {
                    return std::make_shared<const Node::Expression::LogicalNegation>(expression);
                }
                else if (tokenKind == TokenKind::New(TokenKind::BoxKeyword))
                {
                    return std::make_shared<const Node::Expression::Box>(expression);
                }
                else if (tokenKind == TokenKind::New(TokenKind::UnboxKeyword))
                {
                    return std::make_shared<const Node::Expression::Unbox>(expression);
                }
                else
                {
                    return std::make_shared<const Node::Expression::UnaryUser>(
                        expression, 
                        tokenKind
                    );
                }
            }();

            expression = collapsedExpression;
            prefixOperators.pop_back();
        };

        const auto collapseRHS = [&]()
        {
            const auto& [tokenKind, arguments] = postfixOperators.front();

            const auto collapsedExpression = [&tokenKind=tokenKind, &expression=expression, &arguments=arguments]() -> std::shared_ptr<const Node::Expression::IBase>
            {
                if (tokenKind == TokenKind::New(TokenKind::OpenParen))
                {
                    return std::make_shared<const Node::Expression::FunctionCall>(
                        expression,
                        arguments
                    );
                }
                else
                {
                    ACE_UNREACHABLE();
                }
            }();

            expression = collapsedExpression;
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

        return ParseData
        {
            expression,
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseMemberAccessExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::MemberAccess>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(expression, ParsePrimaryExpression({ it, t_context.Scope }));
        it += expression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Dot));
        ++it;

        ACE_TRY(name, ParseSymbolNameSection({ it, t_context.Scope }));
        it += name.Length;

        return ParseData
        {
            std::make_shared<const Node::Expression::MemberAccess>(
                expression.Value,
                name.Value
            ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseArguments(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Expression::IBase>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenParen));
        ++it;

        bool isFirstArgument = true;
        std::vector<std::shared_ptr<const Node::Expression::IBase>> arguments{};
        while (it->Kind != TokenKind::New(TokenKind::CloseParen))
        {
            if (isFirstArgument)
            {
                isFirstArgument = false;
            }
            else
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Comma));
                ++it;
            }

            ACE_TRY(argument, ParseExpression({ it, t_context.Scope }));
            arguments.push_back(argument.Value);
            it += argument.Length;
        }

        ++it;

        return ParseData
        {
            arguments,
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParsePrimaryExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::IBase>>>
    {
        if (const auto expExpressionExpression = ParseExpressionExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                expExpressionExpression.Unwrap().Value,
                expExpressionExpression.Unwrap().Length
            };
        }

        if (const auto expStructConstructionExpression = ParseStructConstructionExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                expStructConstructionExpression.Unwrap().Value,
                expStructConstructionExpression.Unwrap().Length
            };
        }

        if (const auto expLiteralExpression = ParseLiteralExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                expLiteralExpression.Unwrap().Value,
                expLiteralExpression.Unwrap().Length
            };
        }

        if (const auto expLiteralSymbolExpression = ParseLiteralSymbolExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                expLiteralSymbolExpression.Unwrap().Value,
                expLiteralSymbolExpression.Unwrap().Length
            };
        }

        if (const auto castExpression = ParseCastExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                castExpression.Unwrap().Value,
                castExpression.Unwrap().Length
            };
        }

        if (const auto addressOfExpression = ParseAddressOfExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                addressOfExpression.Unwrap().Value,
                addressOfExpression.Unwrap().Length
            };
        }

        if (const auto sizeOfExpression = ParseSizeOfExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                sizeOfExpression.Unwrap().Value,
                sizeOfExpression.Unwrap().Length
            };
        }

        if (const auto derefAsExpression = ParseDerefAsExpression(t_context))
        {
            return ParseData<std::shared_ptr<const Node::Expression::IBase>>
            {
                derefAsExpression.Unwrap().Value,
                derefAsExpression.Unwrap().Length
            };
        }
        
        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseExpressionExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Expression>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenParen));
        ++it;

        ACE_TRY(expression, ParseExpression({ it, t_context.Scope }));
        it += expression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CloseParen));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Expression::Expression>(expression.Value),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseLiteralExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Literal>>>
    {
        ACE_TRY(literalKind, [&]() -> Expected<LiteralKind>
        {
            const auto& tokenKind = t_context.Iterator->Kind;

            if (tokenKind == TokenKind::New(TokenKind::Int8))           return LiteralKind::Int8;
            if (tokenKind == TokenKind::New(TokenKind::Int16))          return LiteralKind::Int16;
            if (tokenKind == TokenKind::New(TokenKind::Int32))          return LiteralKind::Int32;
            if (tokenKind == TokenKind::New(TokenKind::Int64))          return LiteralKind::Int64;
            if (tokenKind == TokenKind::New(TokenKind::UInt8))          return LiteralKind::UInt8;
            if (tokenKind == TokenKind::New(TokenKind::UInt16))         return LiteralKind::UInt16;
            if (tokenKind == TokenKind::New(TokenKind::UInt32))         return LiteralKind::UInt32;
            if (tokenKind == TokenKind::New(TokenKind::UInt64))         return LiteralKind::UInt64;
            if (tokenKind == TokenKind::New(TokenKind::Int))            return LiteralKind::Int;
            if (tokenKind == TokenKind::New(TokenKind::Float32))        return LiteralKind::Float32;
            if (tokenKind == TokenKind::New(TokenKind::Float64))        return LiteralKind::Float64;
            if (tokenKind == TokenKind::New(TokenKind::String))         return LiteralKind::String;
            if (tokenKind == TokenKind::New(TokenKind::TrueKeyword))    return LiteralKind::True;
            if (tokenKind == TokenKind::New(TokenKind::FalseKeyword))   return LiteralKind::False;

            ACE_TRY_UNREACHABLE();
        }());

        return ParseData
        {
            std::make_shared<const Node::Expression::Literal>(
                t_context.Scope,
                literalKind,
                t_context.Iterator->String
            ),
            1
        };
    }

    auto Parser::ParseLiteralSymbolExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::LiteralSymbol>>>
    {
        ACE_TRY(name, ParseSymbolName({ t_context.Iterator, t_context.Scope }));

        return ParseData
        {
            std::make_shared<const Node::Expression::LiteralSymbol>(
                t_context.Scope,
                name.Value
            ),
            name.Length
        };
    }

    auto Parser::ParseStructConstructionExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::StructConstruction>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Identifier));
        ACE_TRY_ASSERT(it->String == SpecialIdentifier::New);
        ++it;

        ACE_TRY(typeName, ParseSymbolName({ it, t_context.Scope }));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBrace));
        ++it;

        std::vector<Node::Expression::StructConstruction::Argument> arguments{};
        while (it->Kind != TokenKind::New(TokenKind::CloseBrace))
        {
            if (arguments.size() != 0)
            {
                ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::Comma));
                ++it;

                if (it->Kind == TokenKind::New(TokenKind::CloseBrace))
                    break;
            }

            ACE_TRY(name, ParseName({ it, t_context.Scope }));
            it += name.Length;

            std::optional<std::shared_ptr<const Node::Expression::IBase>> optValue{};
            if (it->Kind == TokenKind::New(TokenKind::Colon))
            {
                ++it;

                ACE_TRY(value, ParseExpression({ it, t_context.Scope }));
                optValue = value.Value;
                it += value.Length;
            }
            
            arguments.push_back(Node::Expression::StructConstruction::Argument{
                name.Value,
                optValue
            });
        }

        ++it;

        return ParseData
        {
            std::make_shared<const Node::Expression::StructConstruction>(
                t_context.Scope,
                typeName.Value,
                std::move(arguments)
                ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseCastExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::Cast>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CastKeyword));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBracket));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, true));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CloseBracket));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenParen));
        ++it;

        ACE_TRY(expression, ParseExpression({ it, t_context.Scope }));
        it += expression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CloseParen));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Expression::Cast>(
                typeName.Value,
                expression.Value
            ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseAddressOfExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::AddressOf>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::AddressOfKeyword));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenParen));
        ++it;

        ACE_TRY(expression, ParseExpression({ it, t_context.Scope }));
        it += expression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CloseParen));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Expression::AddressOf>(expression.Value),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseSizeOfExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::SizeOf>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::SizeOfKeyword));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBracket));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, true));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CloseBracket));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Expression::SizeOf>(
                t_context.Scope,
                typeName.Value
            ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseDerefAsExpression(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Expression::DerefAs>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::DerefAsKeyword));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBracket));
        ++it;

        ACE_TRY(typeName, ParseTypeName({ it, t_context.Scope }, true));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CloseBracket));
        ++it;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenParen));
        ++it;

        ACE_TRY(expression, ParseExpression({ it, t_context.Scope }));
        it += expression.Length;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::CloseParen));
        ++it;

        return ParseData
        {
            std::make_shared<const Node::Expression::DerefAs>(
                typeName.Value,
                expression.Value
            ),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseAttribute(Context t_context) -> Expected<ParseData<std::shared_ptr<const Node::Attribute>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Kind == TokenKind::New(TokenKind::OpenBracket));
        ++it;

        ACE_TRY(structConstructionExpression, ParseStructConstructionExpression({ it, t_context.Scope }));
        it += structConstructionExpression.Length;

        return ParseData
        {
            std::make_shared<const Node::Attribute>(structConstructionExpression.Value),
            Distance(t_context.Iterator, it)
        };
    }

    auto Parser::ParseAttributes(Context t_context) -> Expected<ParseData<std::vector<std::shared_ptr<const Node::Attribute>>>>
    {
        auto it = t_context.Iterator;

        std::vector<std::shared_ptr<const Node::Attribute>> attributes{};
        while (const auto expAttribute = ParseAttribute({ it, t_context.Scope }))
        {
            attributes.push_back(expAttribute.Unwrap().Value);
            it += expAttribute.Unwrap().Length;
        }

        return ParseData
        {
            attributes,
            Distance(t_context.Iterator, it)
        };
    }
}
