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
#include "Nodes/All.hpp"
#include "Diagnostics.hpp"
#include "Assert.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Utility.hpp"
#include "Name.hpp"
#include "FileBuffer.hpp"
#include "Compilation.hpp"
#include "Measured.hpp"

namespace Ace
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

    ParseToken::ParseToken(
        const std::shared_ptr<const Token>& t_value
    ) : m_Value{ t_value }
    {
    }

    auto ParseToken::Unwrap() const -> const Token&
    {
        return *m_Value.get();
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

    auto Parser::ParseAST(
        const FileBuffer* const t_fileBuffer,
        const std::vector<std::shared_ptr<const Token>>& t_tokens
    ) -> Expected<std::shared_ptr<const ModuleNode>>
    {
        std::vector<ParseToken> tokens{};

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
            [](const std::shared_ptr<const Token>& t_token) { return t_token; }
        );

        tokens.insert(
            end(tokens) - 1,
            std::make_shared<const Token>(
                trailingSourceLocation,
                TokenKind::CloseBrace
            )
        );

        auto it = begin(tokens);

        ACE_TRY(module, ParseModule({
            it,
            t_fileBuffer->GetCompilation()->GlobalScope.Unwrap()
        }));

        it += module.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::EndOfFile);
        ++it;

        ACE_TRY_ASSERT(it == end(tokens));

        return module.Value;
    }

    auto Parser::CreateEmptyAttributes() -> std::vector<std::shared_ptr<const AttributeNode>>
    {
        return {};
    }

    auto Parser::GetOperatorFunctionName(
        const std::shared_ptr<const Token>& t_operatorToken,
        const size_t t_paramCount
    ) -> Expected<const char*>
    {
        const auto& tokenKind = t_operatorToken->Kind;
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

    auto Parser::ParseName(
        const ParseContext& t_context
    ) -> Expected<Measured<std::string>>
    {
        ACE_TRY_ASSERT(t_context.Iterator->Unwrap().Kind == TokenKind::Identifier);

        return Measured
        {
            t_context.Iterator->Unwrap().String,
            1,
        };
    }

    auto Parser::ParseNestedName(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::string>>>
    {
        std::vector<std::string> name{};
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Identifier);
        name.push_back(it->Unwrap().String);
        ++it;

        while (it->Unwrap().Kind == TokenKind::ColonColon)
        {
            ++it;

            ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Identifier);
            name.push_back(it->Unwrap().String);
            ++it;
        }

        return Measured
        {
            std::move(name),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseSymbolName(
        const ParseContext& t_context
    ) -> Expected<Measured<SymbolName>>
    {
        auto it = t_context.Iterator;

        auto resolutionScope = SymbolNameResolutionScope::Local;
        if (it->Unwrap().Kind == TokenKind::ColonColon)
        {
            resolutionScope = SymbolNameResolutionScope::Global;
            ++it;
        }
        
        std::vector<SymbolNameSection> sections{};

        ACE_TRY(section, ParseSymbolNameSection({ it, t_context.Scope }));
        sections.push_back(std::move(section.Value));
        it += section.Length;

        while (it->Unwrap().Kind == TokenKind::ColonColon)
        {
            ++it;

            ACE_TRY(section, ParseSymbolNameSection({ it, t_context.Scope }));
            sections.push_back(std::move(section.Value));
            it += section.Length;
        }

        return Measured
        {
            SymbolName 
            { 
                sections,
                resolutionScope,
            },
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseSymbolNameSection(
        const ParseContext& t_context
    ) -> Expected<Measured<SymbolNameSection>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Identifier);
        const std::string& name = it->Unwrap().String;
        ++it;

        auto templateArgs = [&]() -> std::vector<SymbolName>
        {
            auto expTemplateArgs = ParseTemplateArgs({ it, t_context.Scope });
            if (!expTemplateArgs)
                return {};

            it += expTemplateArgs.Unwrap().Length;
            return expTemplateArgs.Unwrap().Value;
        }();

        return Measured
        {
            SymbolNameSection 
            {
                name,
                templateArgs,
            },
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseTypeName(
        const ParseContext& t_context,
        const ReferenceParsingKind t_referenceParsingKind
    ) -> Expected<Measured<TypeName>>
    {
        auto it = t_context.Iterator;

        std::vector<TypeNameModifier> modifiers{};

        if (t_referenceParsingKind == ReferenceParsingKind::Allow)
        {
            if (it->Unwrap().Kind == TokenKind::Ampersand)
            {
                modifiers.push_back(TypeNameModifier::Reference);
                ++it;
            }
        }

        for (; true; ++it)
        {
            if (it->Unwrap().Kind == TokenKind::Asterisk)
            {
                modifiers.push_back(TypeNameModifier::StrongPointer);
            }
            else if (it->Unwrap().Kind == TokenKind::Tilde)
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

        return Measured
        {
            TypeName
            {
                symbolName.Value,
                modifiers,
            },
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseTemplateParamNames(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::string>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBracket);
        ++it;

        std::vector<std::string> names{};
        while (it->Unwrap().Kind != TokenKind::CloseBracket)
        {
            if (names.size() != 0)
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Comma);
                ++it;
            }

            ACE_TRY(name, ParseName({ it, t_context.Scope }));
            names.push_back(std::move(name.Value));
            it += name.Length;
        }

        ACE_TRY_ASSERT(names.size() != 0);
        ++it;

        return Measured
        {
            std::move(names),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseImplTemplateParams(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const ImplTemplateParamNode>>>>
    {
        ACE_TRY(names, ParseTemplateParamNames(t_context));
        
        std::vector<std::shared_ptr<const ImplTemplateParamNode>> params{};
        std::transform(
            begin(names.Value),
            end  (names.Value),
            back_inserter(params),
            [&](const std::string& t_name)
            {
                return std::make_shared<const ImplTemplateParamNode>(
                    t_context.Scope,
                    t_name
                );
            }
        );

        return Measured
        {
             params,
             names.Length,
        };
    }

    auto Parser::ParseTemplateParams(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>>
    {
        ACE_TRY(names, ParseTemplateParamNames(t_context));
        
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> params{};
        std::transform(
            begin(names.Value),
            end  (names.Value),
            back_inserter(params),
            [&](const std::string& t_name)
            {
                return std::make_shared<const NormalTemplateParamNode>(
                    t_context.Scope,
                    t_name
                );
            }
        );

        return Measured
        {
             params,
             names.Length,
        };
    }

    auto Parser::ParseTemplateArgs(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<SymbolName>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBracket);
        ++it;

        std::vector<SymbolName> args{};
        while (it->Unwrap().Kind != TokenKind::CloseBracket)
        {
            if (args.size() != 0)
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Comma);
                ++it;
            }

            ACE_TRY(arg, ParseSymbolName({ it, t_context.Scope }));
            args.push_back(std::move(arg.Value));
            it += arg.Length;
        }

        ACE_TRY_ASSERT(args.size() != 0);
        ++it;

        return Measured
        {
            std::move(args),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseModule(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const ModuleNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(name, ParseNestedName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::ModuleKeyword);
        ++it;

        auto accessModifier = AccessModifier::Private;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
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

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBrace);
        ++it;

        std::vector<std::shared_ptr<const ModuleNode>> modules{};
        std::vector<std::shared_ptr<const ITypeNode>> types{};
        std::vector<std::shared_ptr<const TypeTemplateNode>> typeTemplates{};
        std::vector<std::shared_ptr<const ImplNode>> impls{};
        std::vector<std::shared_ptr<const TemplatedImplNode>> templatedImpls{};
        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};
        std::vector<std::shared_ptr<const StaticVarNode>> variables{};

        while (it->Unwrap().Kind != TokenKind::CloseBrace)
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

            if (const auto expVar = ParseVar({ it, selfScope }))
            {
                variables.push_back(expVar.Unwrap().Value);
                it += expVar.Unwrap().Length;
                continue;
            }

            ACE_TRY_UNREACHABLE();
        }

        ++it;

        return Measured
        {
            std::make_shared<const ModuleNode>(
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
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseImpl(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const ImplNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::ImplKeyword);
        ++it;

        ACE_TRY(typeName, ParseSymbolName({ it, t_context.Scope }));

        // TODO: Remove this block after impl template specialization.
        {
            const bool foundTemplatedSection = std::find_if(begin(typeName.Value.Sections), end(typeName.Value.Sections),
            [](const SymbolNameSection& t_section)
            {
                return t_section.TemplateArgs.empty();
            }) == end(typeName.Value.Sections);

            ACE_TRY_ASSERT(!foundTemplatedSection);
        }

        it += typeName.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBrace);
        ++it;

        std::vector<std::shared_ptr<const FunctionNode>> functions{};
        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};

        while (it->Unwrap().Kind != TokenKind::CloseBrace)
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

        return Measured
        {
            std::make_shared<const ImplNode>(
                scope,
                typeName.Value,
                functions,
                functionTemplates
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseImplFunction(
        const ParseContext& t_context,
        const SymbolName& t_selfTypeName
    ) -> Expected<Measured<std::shared_ptr<const FunctionNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(nameToken, [&]() -> Expected<std::shared_ptr<const Token>>
        {
            if (it->Unwrap().Kind == TokenKind::OperatorKeyword) 
            {
                ++it;

                const std::shared_ptr<const Token> operatorToken = *it;

                if (!IsUserOperator(operatorToken->Kind))
                {
                    ACE_TRY_ASSERT(
                        operatorToken->Kind == TokenKind::ImplKeyword ||
                        operatorToken->Kind == TokenKind::ExplKeyword ||
                        operatorToken->Kind == TokenKind::Identifier 
                    );
                }

                ++it;

                return operatorToken;
            }
            else
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Identifier);
                const std::shared_ptr<const Token> nameToken = *it;
                ++it;

                return nameToken;
            }
        }());

        ACE_TRY(params, ParseParams({ it, scope }));
        it += params.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Disallow
        ));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasSelfModifier = false;
        bool hasExternModifier = false;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Unwrap().Kind == TokenKind::Identifier)
            {
                ACE_TRY_ASSERT(it->Unwrap().String == SpecialIdentifier::Self);
                hasSelfModifier = true;
                ++it;
            }

            if (it->Unwrap().Kind == TokenKind::ExternKeyword)
            {
                hasExternModifier = true;
                ACE_TRY_ASSERT(!hasSelfModifier);
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(name, [&]() -> Expected<std::string>
        {
            if (nameToken->Kind == TokenKind::Identifier)
            {
                return nameToken->String;
            }
            else
            {
                ACE_TRY(name, GetOperatorFunctionName(
                    nameToken,
                    params.Value.size()
                ));

                ACE_TRY_ASSERT(accessModifier == AccessModifier::Public);
                ACE_TRY_ASSERT(!hasSelfModifier);

                return std::string{ name };
            }
        }());

        ACE_TRY(optBody, [&]() -> Expected<std::optional<std::shared_ptr<const BlockStmtNode>>>
        {
            if (hasExternModifier)
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
                ++it;

                return std::optional<std::shared_ptr<const BlockStmtNode>>{};
            }
            else
            {
                ACE_TRY(body, ParseBlockStmt({ it, scope }));
                it += body.Length;

                return std::optional{ body.Value };
            }
        }());

        const auto selfParam = [&]() -> std::optional<std::shared_ptr<const SelfParamVarNode>>
        {
            if (!hasSelfModifier)
            {
                return std::nullopt;
            }

            return std::make_shared<const SelfParamVarNode>(
                scope,
                t_selfTypeName
            );
        }();

        return Measured
        {
            std::make_shared<const FunctionNode>(
                scope,
                name,
                typeName.Value,
                attributes.Value,
                accessModifier,
                selfParam,
                params.Value,
                optBody
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseImplFunctionTemplate(
        const ParseContext& t_context,
        const SymbolName& t_selfTypeName
    ) -> Expected<Measured<std::shared_ptr<const FunctionTemplateNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(templateParams, ParseTemplateParams({ it, scope }));
        it += templateParams.Length;

        ACE_TRY(params, ParseParams({ it, scope }));
        it += params.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Disallow
        ));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasSelfModifier = false;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Unwrap().Kind == TokenKind::Identifier)
            {
                ACE_TRY_ASSERT(it->Unwrap().String == SpecialIdentifier::Self);
                hasSelfModifier = true;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseBlockStmt({ it, scope }));
        it += body.Length;

        const auto selfParam = [&]() -> std::optional<std::shared_ptr<const SelfParamVarNode>>
        {
            if (!hasSelfModifier)
            {
                return std::nullopt;
            }

            return std::make_shared<const SelfParamVarNode>(
                scope,
                t_selfTypeName
            );
        }();

        const auto function = std::make_shared<const FunctionNode>(
            scope,
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            selfParam,
            params.Value,
            body.Value
        );

        return Measured
        {
            std::make_shared<const FunctionTemplateNode>(
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                templateParams.Value,
                function
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseTemplatedImpl(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const TemplatedImplNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::ImplKeyword);
        ++it;

        ACE_TRY(templateParams, ParseImplTemplateParams({ it, scope }));
        it += templateParams.Length;

        ACE_TRY(typeName, ParseSymbolName({ it, t_context.Scope }));

        // TODO: Remove this block after impl template specialization.
        {
            const bool foundTemplatedSection = std::find_if(begin(typeName.Value.Sections), end(typeName.Value.Sections) - 1,
            [](const SymbolNameSection& t_section)
            {
                return t_section.TemplateArgs.empty();
            }) == end(typeName.Value.Sections);

            ACE_TRY_ASSERT(!foundTemplatedSection);

            const auto& templateArgs = typeName.Value.Sections.back().TemplateArgs;
            ACE_TRY_ASSERT(templateParams.Value.size() == templateArgs.size());

            std::unordered_set<std::string> templateParamSet{};
            ACE_TRY_VOID(TransformExpectedVector(templateParams.Value,
            [&](const std::shared_ptr<const ImplTemplateParamNode>& t_templateParam) -> Expected<void>
            {
                const std::string& templateParamName = t_templateParam->GetName();
                ACE_TRY_ASSERT(!templateParamSet.contains(templateParamName));
                templateParamSet.insert(templateParamName);

                return Void{};
            }));
            
            ACE_TRY_VOID(TransformExpectedVector(templateArgs,
            [&](const SymbolName& t_arg) -> Expected<void>
            {
                ACE_TRY_ASSERT(t_arg.Sections.size() == 1);
                ACE_TRY_ASSERT(t_arg.Sections.back().TemplateArgs.empty());

                const std::string& templateArgName = t_arg.Sections.front().Name;
                ACE_TRY_ASSERT(templateParamSet.contains(templateArgName));
                templateParamSet.erase(templateArgName);

                return Void{};
            }));
        }

        it += typeName.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBrace);
        ++it;

        std::vector<std::shared_ptr<const FunctionTemplateNode>> functionTemplates{};

        while (it->Unwrap().Kind != TokenKind::CloseBrace)
        {
            auto expFunctionTemplate = ParseTemplatedImplFunction(
                { it, scope },
                typeName.Value,
                templateParams.Value
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
        typeTemplateNameLastSection.TemplateArgs.clear();
        typeTemplateNameLastSection.Name = SpecialIdentifier::CreateTemplate(
            typeTemplateNameLastSection.Name
        );

        return Measured
        {
            std::make_shared<const TemplatedImplNode>(
                scope,
                typeTemplateName,
                std::vector<std::shared_ptr<const FunctionNode>>{},
                functionTemplates
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseTemplatedImplFunction(
        const ParseContext& t_context,
        const SymbolName& t_selfTypeName,
        const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& t_implTemplateParams
    ) -> Expected<Measured<std::shared_ptr<const FunctionTemplateNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(nameToken, [&]() -> Expected<std::shared_ptr<const Token>>
        {
            if (it->Unwrap().Kind == TokenKind::OperatorKeyword)
            {
                ++it;

                const std::shared_ptr<const Token> operatorToken = *it;

                if (!IsUserOperator(operatorToken->Kind))
                {
                    ACE_TRY_ASSERT(
                        operatorToken->Kind == TokenKind::ImplKeyword ||
                        operatorToken->Kind == TokenKind::ExplKeyword ||
                        operatorToken->Kind == TokenKind::Identifier
                    );
                }

                ++it;

                return operatorToken;
            }
            else
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Identifier);
                const std::shared_ptr<const Token> nameToken = *it;
                ++it;

                return nameToken;
            }
        }());

        ACE_TRY(templateParams, [&]() -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
        {
            if (const auto expTemplateParams = ParseTemplateParams({ it, scope }))
            {
                it += expTemplateParams.Unwrap().Length;
                return expTemplateParams.Unwrap().Value;
            }
            else
            {
                return std::vector<std::shared_ptr<const NormalTemplateParamNode>>{};
            }
        }());

        ACE_TRY(params, ParseParams({ it, scope }));
        it += params.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Disallow
        ));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasSelfModifier = false;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Unwrap().Kind == TokenKind::Identifier)
            {
                ACE_TRY_ASSERT(it->Unwrap().String == SpecialIdentifier::Self);
                hasSelfModifier = true;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(name, [&]() -> Expected<std::string>
        {
            if (nameToken->Kind == TokenKind::Identifier)
            {
                return nameToken->String;
            }
            else
            {
                ACE_TRY(name, GetOperatorFunctionName(
                    nameToken,
                    params.Value.size()
                ));

                ACE_TRY_ASSERT(accessModifier == AccessModifier::Public);
                ACE_TRY_ASSERT(!hasSelfModifier);

                return std::string{ name };
            }
        }());

        ACE_TRY(body, ParseBlockStmt({ it, scope }));
        it += body.Length;

        const auto selfParam = [&]() -> std::optional<std::shared_ptr<const SelfParamVarNode>>
        {
            if (!hasSelfModifier)
            {
                return std::nullopt;
            }

            return std::make_shared<const SelfParamVarNode>(
                scope,
                t_selfTypeName
            );
        }();

        const auto function = std::make_shared<const FunctionNode>(
            scope,
            name,
            typeName.Value,
            attributes.Value,
            accessModifier,
            selfParam,
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


        return Measured
        {
            std::make_shared<const FunctionTemplateNode>(
                clonedImplTemplateParams,
                templateParams,
                function
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseFunction(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const FunctionNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(params, ParseParams({ it, scope }));
        it += params.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Disallow
        ));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;
        bool hasExternModifier = false;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            if (it->Unwrap().Kind == TokenKind::ExternKeyword)
            {
                hasExternModifier = true;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(optBody, [&]() -> Expected<std::optional<std::shared_ptr<const BlockStmtNode>>>
        {
            if (hasExternModifier)
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
                ++it;

                return std::optional<std::shared_ptr<const BlockStmtNode>>{};
            }
            else
            {
                ACE_TRY(body, ParseBlockStmt({ it, scope }));
                it += body.Length;

                return std::optional{ body.Value };
            }
        }());

        return Measured
        {
            std::make_shared<const FunctionNode>(
                scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                accessModifier,
                std::nullopt,
                params.Value,
                optBody
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseFunctionTemplate(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const FunctionTemplateNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(templateParams, [&]() -> Expected<std::vector<std::shared_ptr<const NormalTemplateParamNode>>>
        {
            const auto expTemplateParams = ParseTemplateParams({
                it,
                scope,
            });
            if (expTemplateParams)
            {
                it += expTemplateParams.Unwrap().Length;
                return expTemplateParams.Unwrap().Value;
            }
            else
            {
                return std::vector<std::shared_ptr<const NormalTemplateParamNode>>{};
            }
        }());

        ACE_TRY(params, ParseParams({ it, scope }));
        it += params.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Disallow
        ));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseBlockStmt({ it, scope }));
        it += body.Length;

        const auto function = std::make_shared<const FunctionNode>(
            scope,
            name.Value,
            typeName.Value,
            attributes.Value,
            accessModifier,
            std::nullopt,
            params.Value,
            body.Value
        );

        return Measured
        {
            std::make_shared<const FunctionTemplateNode>(
                std::vector<std::shared_ptr<const ImplTemplateParamNode>>{},
                templateParams,
                function
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseParams(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const NormalParamVarNode>>>>
    {
        std::vector<std::shared_ptr<const NormalParamVarNode>> params{};
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenParen);
        ++it;

        bool isFirstParam = true;
        while (it->Unwrap().Kind != TokenKind::CloseParen)
        {
            if (isFirstParam)
            {
                isFirstParam = false;
            }
            else
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Comma);
                ++it;
            }

            ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
            it += attributes.Length;

            ACE_TRY(name, ParseName({ it, t_context.Scope }));
            it += name.Length;

            ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
            ++it;

            ACE_TRY(typeName, ParseTypeName(
                { it, t_context.Scope },
                ReferenceParsingKind::Allow
            ));
            it += typeName.Length;

            params.push_back(std::make_shared<const NormalParamVarNode>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                params.size()
            ));
        }

        ++it;

        return Measured
        {
            params,
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseVar(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const StaticVarNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Disallow
        ));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
        ++it;

        return Measured
        {
            std::make_shared<const StaticVarNode>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                accessModifier
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseMemberVar(
        const ParseContext& t_context,
        const size_t t_index
    ) -> Expected<Measured<std::shared_ptr<const InstanceVarNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Disallow
        ));
        it += typeName.Length;

        auto accessModifier = AccessModifier::Private;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        return Measured
        {
            std::make_shared<const InstanceVarNode>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                attributes.Value,
                accessModifier,
                t_index
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseType(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const ITypeNode>>>
    {
        if (const auto expStruct = ParseStruct({ t_context.Iterator, t_context.Scope }))
        {
            return Measured<std::shared_ptr<const ITypeNode>>
            {
                expStruct.Unwrap().Value,
                expStruct.Unwrap().Length,
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseTypeTemplate(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const TypeTemplateNode>>>
    {
        if (const auto expStructTemplate = ParseStructTemplate({ t_context.Iterator, t_context.Scope }))
        {
            return expStructTemplate;
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseStruct(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const StructTypeNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::StructKeyword);
        ++it;

        auto accessModifier = AccessModifier::Private;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseStructBody({ it, scope }));
        it += body.Length;

        return Measured
        {
            std::make_shared<const StructTypeNode>(
                scope,
                name.Value,
                attributes.Value,
                accessModifier,
                body.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseStructBody(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const InstanceVarNode>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBrace);
        ++it;

        std::vector<std::shared_ptr<const InstanceVarNode>> variables{};
        while (it->Unwrap().Kind != TokenKind::CloseBrace)
        {
            if (variables.size() != 0)
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Comma);
                ++it;

                if (it->Unwrap().Kind == TokenKind::CloseBrace)
                    break;
            }

            ACE_TRY(variable, ParseMemberVar({ it, t_context.Scope }, variables.size()));
            variables.push_back(variable.Value);
            it += variable.Length;
        }

        ++it;

        return Measured<std::vector<std::shared_ptr<const InstanceVarNode>>>
        {
            variables,
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseStructTemplate(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const TypeTemplateNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY(attributes, ParseAttributes({ it, t_context.Scope }));
        it += attributes.Length;

        ACE_TRY(name, ParseName({ it, t_context.Scope }));
        it += name.Length;

        ACE_TRY(templateParams, ParseTemplateParams({ it, scope }));
        it += templateParams.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::StructKeyword);
        ++it;

        auto accessModifier = AccessModifier::Private;

        if (it->Unwrap().Kind == TokenKind::MinusGreaterThan)
        {
            ++it;
            const auto startIt = it;

            if (it->Unwrap().Kind == TokenKind::PublicKeyword)
            {
                accessModifier = AccessModifier::Public;
                ++it;
            }

            ACE_TRY_ASSERT(it != startIt);
        }

        ACE_TRY(body, ParseStructBody({ it, scope }));
        it += body.Length;

        const auto type = std::make_shared<const StructTypeNode>(
            scope,
            name.Value,
            attributes.Value,
            accessModifier,
            body.Value
        );

        return Measured
        {
            std::make_shared<const TypeTemplateNode>(
                templateParams.Value,
                type
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const IStmtNode>>>
    {
        if (const auto expExprStatemment = ParseExprStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expExprStatemment.Unwrap().Value,
                expExprStatemment.Unwrap().Length,
            };
        }

        if (const auto expAssignmentStmt = ParseAssignmentStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expAssignmentStmt.Unwrap().Value,
                expAssignmentStmt.Unwrap().Length,
            };
        }

        if (const auto expCompoundAssignmentStmt = ParseCompoundAssignmentStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expCompoundAssignmentStmt.Unwrap().Value,
                expCompoundAssignmentStmt.Unwrap().Length,
            };
        }

        if (const auto expVarStmt = ParseVarStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expVarStmt.Unwrap().Value,
                expVarStmt.Unwrap().Length,
            };
        }

        if (const auto expKeywordStmt = ParseKeywordStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expKeywordStmt.Unwrap().Value,
                expKeywordStmt.Unwrap().Length,
            };
        }

        if (const auto expCompoundStmt = ParseBlockStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expCompoundStmt.Unwrap().Value,
                expCompoundStmt.Unwrap().Length,
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseExprStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const ExprStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(expr, ParseExpr({ it, t_context.Scope }));
        it += expr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
        ++it;

        return Measured
        {
            std::make_shared<const ExprStmtNode>(expr.Value),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseAssignmentStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const NormalAssignmentStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(lhsExpr, ParseExpr({ it, t_context.Scope }));
        it += lhsExpr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Equals);
        ++it;

        ACE_TRY(rhsExpr, ParseExpr({ it, t_context.Scope }));
        it += rhsExpr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
        ++it;

        return Measured
        {
            std::make_shared<const NormalAssignmentStmtNode>(
                t_context.Scope,
                lhsExpr.Value,
                rhsExpr.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseCompoundAssignmentStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const CompoundAssignmentStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(lhsExpr, ParseExpr({ it, t_context.Scope }));
        it += lhsExpr.Length;

        const auto op = it->Unwrap().Kind;
        ACE_TRY_ASSERT(IsCompoundAssignmentOperator(op));
        ++it;

        ACE_TRY(rhsExpr, ParseExpr({ it, t_context.Scope }));
        it += rhsExpr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
        ++it;

        return Measured
        {
            std::make_shared<const CompoundAssignmentStmtNode>(
                t_context.Scope,
                lhsExpr.Value,
                rhsExpr.Value,
                op
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseVarStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const VarStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(name, ParseName(t_context));
        it += name.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Colon);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Allow
        ));
        it += typeName.Length;

        ACE_TRY(optAssignment, [&]() -> Expected<std::optional<std::shared_ptr<const IExprNode>>>
        {
            if (it->Unwrap().Kind == TokenKind::Semicolon)
            {
                ++it;
                return std::optional<std::shared_ptr<const IExprNode>>{};
            }
            else if (it->Unwrap().Kind == TokenKind::Equals)
            {
                ++it;

                ACE_TRY(expr, ParseExpr({ it, t_context.Scope }));
                it += expr.Length;

                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
                ++it;

                return std::optional{ expr.Value };
            }

            ACE_TRY_UNREACHABLE();
        }());

        return Measured
        {
            std::make_shared<const VarStmtNode>(
                t_context.Scope,
                name.Value,
                typeName.Value,
                optAssignment
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseKeywordStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const IStmtNode>>>
    {
        if (const auto expIfStmt = ParseIfStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expIfStmt.Unwrap().Value,
                expIfStmt.Unwrap().Length,
            };
        }

        if (const auto expWhileStmt = ParseWhileStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expWhileStmt.Unwrap().Value,
                expWhileStmt.Unwrap().Length,
            };
        }

        if (const auto expReturnStmt = ParseReturnStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expReturnStmt.Unwrap().Value,
                expReturnStmt.Unwrap().Length,
            };
        }

        if (const auto expExitStmt = ParseExitStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expExitStmt.Unwrap().Value,
                expExitStmt.Unwrap().Length,
            };
        }

        if (const auto expAssertStmt = ParseAssertStmt(t_context))
        {
            return Measured<std::shared_ptr<const IStmtNode>>
            {
                expAssertStmt.Unwrap().Value,
                expAssertStmt.Unwrap().Length,
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseIfStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const IfStmtNode>>>
    {
        auto it = t_context.Iterator;

        std::vector<std::shared_ptr<const IExprNode>> conditions{};
        std::vector<std::shared_ptr<const BlockStmtNode>> bodies{};

        ACE_TRY(ifBlock, ParseIfBlock({ it, t_context.Scope }));
        it += ifBlock.Length;

        conditions.push_back(ifBlock.Value.first);
        bodies.push_back(ifBlock.Value.second);

        while (it->Unwrap().Kind == TokenKind::ElifKeyword)
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

        return Measured
        {
            std::make_shared<const IfStmtNode>(
                t_context.Scope,
                conditions,
                bodies
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseIfBlock(
        const ParseContext& t_context
    ) -> Expected<Measured<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::IfKeyword);
        ++it;

        ACE_TRY(condition, ParseExpr({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY(body, ParseBlockStmt({ it, t_context.Scope }));
        it += body.Length;

        return Measured
        {
            std::pair
            {
                condition.Value,
                body.Value
            },
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseElifBlock(
        const ParseContext& t_context
    ) -> Expected<Measured<std::pair<std::shared_ptr<const IExprNode>, std::shared_ptr<const BlockStmtNode>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::ElifKeyword);
        ++it;

        ACE_TRY(condition, ParseExpr({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY(body, ParseBlockStmt({ it, t_context.Scope }));
        it += body.Length;

        return Measured
        {
            std::pair
            {
                condition.Value,
                body.Value
            },
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseElseBlock(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const BlockStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::ElseKeyword);
        ++it;

        ACE_TRY(body, ParseBlockStmt({ it, t_context.Scope }));
        it += body.Length;

        return Measured
        {
            body.Value,
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseWhileStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const WhileStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::WhileKeyword);
        ++it;

        ACE_TRY(condition, ParseExpr({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY(body, ParseBlockStmt({ it, t_context.Scope }));
        it += body.Length;

        return Measured
        {
            std::make_shared<const WhileStmtNode>(
                t_context.Scope,
                condition.Value,
                body.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseReturnStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const ReturnStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::ReturnKeyword);
        ++it;

        ACE_TRY(optExpr, [&]() -> Expected<std::optional<std::shared_ptr<const IExprNode>>>
        {
            if (it->Unwrap().Kind == TokenKind::Semicolon)
            {
                ++it;
                return std::optional<std::shared_ptr<const IExprNode>>{};
            }

            ACE_TRY(expr, ParseExpr({ it, t_context.Scope }));
            it += expr.Length;

            ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
            ++it;

            return std::optional{ expr.Value };
        }());

        return Measured
        {
            std::make_shared<const ReturnStmtNode>(
                t_context.Scope,
                optExpr
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseExitStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const ExitStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::ExitKeyword);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
        ++it;

        return Measured
        {
            std::make_shared<const ExitStmtNode>(t_context.Scope),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseAssertStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const AssertStmtNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::AssertKeyword);
        ++it;

        ACE_TRY(condition, ParseExpr({ it, t_context.Scope }));
        it += condition.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Semicolon);
        ++it;

        return Measured
        {
            std::make_shared<const AssertStmtNode>(
                t_context.Scope,
                condition.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseBlockStmt(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const BlockStmtNode>>>
    {
        const auto scope = t_context.Scope->GetOrCreateChild({});
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBrace);
        ++it;

        std::vector<std::shared_ptr<const IStmtNode>> stmts{};
        while (it->Unwrap().Kind != TokenKind::CloseBrace)
        {
            ACE_TRY(stmt, ParseStmt({ it, scope }));
            stmts.push_back(stmt.Value);
            it += stmt.Length;
        }

        ++it;

        return Measured
        {
            std::make_shared<const BlockStmtNode>(
                scope,
                stmts
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const IExprNode>>>
    {
        static constexpr size_t binaryOperatorPrecedenceMin = 9;
        static const auto getBinaryOperatorPrecedence = [](const TokenKind t_tokenKind) -> size_t
        {
            if (
                (t_tokenKind == TokenKind::Asterisk) ||
                (t_tokenKind == TokenKind::Slash) ||
                (t_tokenKind == TokenKind::Percent)
                )
            {
                return 0;
            }

            if (
                (t_tokenKind == TokenKind::Plus) ||
                (t_tokenKind == TokenKind::Minus)
                )
            {
                return 1;
            }

            if (
                (t_tokenKind == TokenKind::LessThanLessThan) ||
                (t_tokenKind == TokenKind::GreaterThanGreaterThan)
                )
            {
                return 2;
            }

            if (
                (t_tokenKind == TokenKind::LessThan) ||
                (t_tokenKind == TokenKind::LessThanEquals) ||
                (t_tokenKind == TokenKind::GreaterThan) ||
                (t_tokenKind == TokenKind::GreaterThanEquals)
                )
            {
                return 3;
            }

            if (
                (t_tokenKind == TokenKind::EqualsEquals) ||
                (t_tokenKind == TokenKind::ExclamationEquals)
                )
            {
                return 4;
            }

            if (t_tokenKind == TokenKind::Ampersand)
            {
                return 5;
            }

            if (t_tokenKind == TokenKind::Caret)
            {
                return 6;
            }

            if (t_tokenKind == TokenKind::VerticalBar)
            {
                return 7;
            }

            if (t_tokenKind == TokenKind::AmpersandAmpersand)
            {
                return 8;
            }

            if (t_tokenKind == TokenKind::VerticalBarVerticalBar)
            {
                return binaryOperatorPrecedenceMin;
            }

            ACE_UNREACHABLE();
        };
        static const auto getBinaryOperatorPrecedencesAssociativity = [](const size_t t_precedence) -> OperatorAssociativity
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

        ACE_TRY(simpleExpr, ParseSimpleExpr({ it, t_context.Scope }));
        it += simpleExpr.Length;

        std::vector<std::shared_ptr<const IExprNode>> exprs{};
        std::vector<std::reference_wrapper<const TokenKind>> operators{};

        exprs.push_back(simpleExpr.Value);
        
        while (IsBinaryOperator(it->Unwrap().Kind))
        {
            operators.push_back(it->Unwrap().Kind);
            ++it;

            ACE_TRY(simpleExpr, ParseSimpleExpr({ it, t_context.Scope }));
            exprs.push_back(simpleExpr.Value);
            it += simpleExpr.Length;
        }

        const auto collapseOperator = [&](const size_t t_index)
        {
            const auto& lhsExpr = exprs.at(t_index);
            const auto& rhsExpr = exprs.at(t_index + 1);

            const auto collapsedExpr = [&]() -> std::shared_ptr<const IExprNode>
            {
                const auto& tokenKind = operators.at(t_index).get();

                switch (tokenKind)
                {
                    case TokenKind::AmpersandAmpersand:
                    {
                        return std::make_shared<const AndExprNode>(
                            lhsExpr,
                            rhsExpr
                        );
                    }

                    case TokenKind::VerticalBarVerticalBar:
                    {
                        return std::make_shared<const OrExprNode>(
                            lhsExpr,
                            rhsExpr
                        );
                    }
                     
                    default:
                    {
                        return std::make_shared<const UserBinaryExprNode>(
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
                    break;

                const auto [startOperatorIndex, endOperatorIndex, operatorIndexIncrement] = [&]() -> std::tuple<int, int, int>
                {
                    switch (associativity)
                    {
                        case OperatorAssociativity::LeftToRight:
                        {
                            return
                            {
                                0,
                                static_cast<int>(operators.size()),
                                1,
                            };
                        }

                        case OperatorAssociativity::RightToLeft:
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

        return Measured
        {
            exprs.front(),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseSimpleExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const IExprNode>>>
    {
        static constexpr size_t unaryOperatorPrecedenceMin = 1;
        static const auto getUnaryOperatorPrecedence = [](const TokenKind t_tokenKind) -> size_t
        {
            if (t_tokenKind == TokenKind::OpenParen)
            {
                return 0;
            }

            if (
                (t_tokenKind == TokenKind::Plus) ||
                (t_tokenKind == TokenKind::Minus) ||
                (t_tokenKind == TokenKind::Tilde) ||
                (t_tokenKind == TokenKind::Exclamation) ||
                (t_tokenKind == TokenKind::BoxKeyword) ||
                (t_tokenKind == TokenKind::UnboxKeyword)
                )
            {
                return unaryOperatorPrecedenceMin;
            }

            ACE_UNREACHABLE();
        };

        auto it = t_context.Iterator;

        std::vector<std::reference_wrapper<const TokenKind>> prefixOperators{};
        std::vector<std::pair<std::reference_wrapper<const TokenKind>, std::vector<std::shared_ptr<const IExprNode>>>> postfixOperators{};

        while (IsPrefixOperator(it->Unwrap().Kind))
        {
            const auto& tokenKind = it->Unwrap().Kind;
            ++it;

            prefixOperators.emplace_back(tokenKind);
        }

        ACE_TRY(primaryExpr, ParsePrimaryExpr({ it, t_context.Scope }));

        ACE_TRY(rootExpr, [&]() -> Expected<std::shared_ptr<const IExprNode>>
        {
            if (const auto expMemberAccessExpr = ParseMemberAccessExpr({ it, t_context.Scope }))
            {
                it += expMemberAccessExpr.Unwrap().Length;
                return std::shared_ptr<const IExprNode>
                {
                    expMemberAccessExpr.Unwrap().Value
                };
            }

            it += primaryExpr.Length;
            return primaryExpr.Value;
        }());

        while (IsPostfixOperator(it->Unwrap().Kind))
        {
            const auto& tokenKind = it->Unwrap().Kind;

            ACE_TRY(args, [&]() -> Expected<std::vector<std::shared_ptr<const IExprNode>>>
            {
                if (it->Unwrap().Kind != TokenKind::OpenParen)
                {
                    ++it;
                    return std::vector<std::shared_ptr<const IExprNode>>{};
                }

                ACE_TRY(args, ParseArgs({ it, t_context.Scope }));
                it += args.Length;
                return args.Value;
            }());

            postfixOperators.emplace_back(tokenKind, args);
        }

        auto expr = rootExpr;

        const auto collapseLHS = [&]()
        {
            const auto& tokenKind = prefixOperators.front();

            const auto collapsedExpr = [&]() -> std::shared_ptr<const IExprNode>
            {
                switch (tokenKind)
                {
                    case TokenKind::Exclamation:
                    {
                        return std::make_shared<const LogicalNegationExprNode>(
                            expr
                        );
                    }

                    case TokenKind::BoxKeyword:
                    {
                        return std::make_shared<const BoxExprNode>(
                            expr
                        );
                    }

                    case TokenKind::UnboxKeyword:
                    {
                        return std::make_shared<const UnboxExprNode>(
                            expr
                        );
                    }

                    default:
                    {
                        return std::make_shared<const UserUnaryExprNode>(
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
            const auto& [tokenKind, args] = postfixOperators.front();

            const auto collapsedExpr = [&tokenKind=tokenKind, &expr=expr, &args=args]() -> std::shared_ptr<const IExprNode>
            {
                if (tokenKind == TokenKind::OpenParen)
                {
                    return std::make_shared<const FunctionCallExprNode>(
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

        return Measured
        {
            expr,
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseMemberAccessExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const MemberAccessExprNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY(expr, ParsePrimaryExpr({ it, t_context.Scope }));
        it += expr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Dot);
        ++it;

        ACE_TRY(name, ParseSymbolNameSection({ it, t_context.Scope }));
        it += name.Length;

        return Measured
        {
            std::make_shared<const MemberAccessExprNode>(
                expr.Value,
                name.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseArgs(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const IExprNode>>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenParen);
        ++it;

        bool isFirstArg = true;
        std::vector<std::shared_ptr<const IExprNode>> args{};
        while (it->Unwrap().Kind != TokenKind::CloseParen)
        {
            if (isFirstArg)
            {
                isFirstArg = false;
            }
            else
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Comma);
                ++it;
            }

            ACE_TRY(arg, ParseExpr({ it, t_context.Scope }));
            args.push_back(arg.Value);
            it += arg.Length;
        }

        ++it;

        return Measured
        {
            args,
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParsePrimaryExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const IExprNode>>>
    {
        if (const auto expExprExpr = ParseExprExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                expExprExpr.Unwrap().Value,
                expExprExpr.Unwrap().Length,
            };
        }

        if (const auto expStructConstructionExpr = ParseStructConstructionExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                expStructConstructionExpr.Unwrap().Value,
                expStructConstructionExpr.Unwrap().Length,
            };
        }

        if (const auto expLiteralExpr = ParseLiteralExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                expLiteralExpr.Unwrap().Value,
                expLiteralExpr.Unwrap().Length,
            };
        }

        if (const auto expLiteralSymbolExpr = ParseLiteralSymbolExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                expLiteralSymbolExpr.Unwrap().Value,
                expLiteralSymbolExpr.Unwrap().Length,
            };
        }

        if (const auto castExpr = ParseCastExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                castExpr.Unwrap().Value,
                castExpr.Unwrap().Length,
            };
        }

        if (const auto addressOfExpr = ParseAddressOfExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                addressOfExpr.Unwrap().Value,
                addressOfExpr.Unwrap().Length,
            };
        }

        if (const auto sizeOfExpr = ParseSizeOfExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                sizeOfExpr.Unwrap().Value,
                sizeOfExpr.Unwrap().Length,
            };
        }

        if (const auto derefAsExpr = ParseDerefAsExpr(t_context))
        {
            return Measured<std::shared_ptr<const IExprNode>>
            {
                derefAsExpr.Unwrap().Value,
                derefAsExpr.Unwrap().Length,
            };
        }
        
        ACE_TRY_UNREACHABLE();
    }

    auto Parser::ParseExprExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const ExprExprNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenParen);
        ++it;

        ACE_TRY(expr, ParseExpr({ it, t_context.Scope }));
        it += expr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CloseParen);
        ++it;

        return Measured
        {
            std::make_shared<const ExprExprNode>(expr.Value),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseLiteralExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const LiteralExprNode>>>
    {
        ACE_TRY(literalKind, [&]() -> Expected<LiteralKind>
        {
            const auto& tokenKind = t_context.Iterator->Unwrap().Kind;

            switch (tokenKind)
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

        return Measured
        {
            std::make_shared<const LiteralExprNode>(
                t_context.Scope,
                literalKind,
                t_context.Iterator->Unwrap().String
            ),
            1,
        };
    }

    auto Parser::ParseLiteralSymbolExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const LiteralSymbolExprNode>>>
    {
        ACE_TRY(name, ParseSymbolName({ t_context.Iterator, t_context.Scope }));

        return Measured
        {
            std::make_shared<const LiteralSymbolExprNode>(
                t_context.Scope,
                name.Value
            ),
            name.Length,
        };
    }

    auto Parser::ParseStructConstructionExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const StructConstructionExprNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Identifier);
        ACE_TRY_ASSERT(it->Unwrap().String == SpecialIdentifier::New);
        ++it;

        ACE_TRY(typeName, ParseSymbolName({ it, t_context.Scope }));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBrace);
        ++it;

        std::vector<StructConstructionExprArg> args{};
        while (it->Unwrap().Kind != TokenKind::CloseBrace)
        {
            if (args.size() != 0)
            {
                ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::Comma);
                ++it;

                if (it->Unwrap().Kind == TokenKind::CloseBrace)
                    break;
            }

            ACE_TRY(name, ParseName({ it, t_context.Scope }));
            it += name.Length;

            std::optional<std::shared_ptr<const IExprNode>> optValue{};
            if (it->Unwrap().Kind == TokenKind::Colon)
            {
                ++it;

                ACE_TRY(value, ParseExpr({ it, t_context.Scope }));
                optValue = value.Value;
                it += value.Length;
            }
            
            args.push_back(StructConstructionExprArg{
                name.Value,
                optValue
            });
        }

        ++it;

        return Measured
        {
            std::make_shared<const StructConstructionExprNode>(
                t_context.Scope,
                typeName.Value,
                std::move(args)
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseCastExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const CastExprNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CastKeyword);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBracket);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Allow
        ));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CloseBracket);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenParen);
        ++it;

        ACE_TRY(expr, ParseExpr({ it, t_context.Scope }));
        it += expr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CloseParen);
        ++it;

        return Measured
        {
            std::make_shared<const CastExprNode>(
                typeName.Value,
                expr.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseAddressOfExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const AddressOfExprNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::AddressOfKeyword);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenParen);
        ++it;

        ACE_TRY(expr, ParseExpr({ it, t_context.Scope }));
        it += expr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CloseParen);
        ++it;

        return Measured
        {
            std::make_shared<const AddressOfExprNode>(expr.Value),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseSizeOfExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const SizeOfExprNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::SizeOfKeyword);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBracket);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Allow
        ));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CloseBracket);
        ++it;

        return Measured
        {
            std::make_shared<const SizeOfExprNode>(
                t_context.Scope,
                typeName.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseDerefAsExpr(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const DerefAsExprNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::DerefAsKeyword);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBracket);
        ++it;

        ACE_TRY(typeName, ParseTypeName(
            { it, t_context.Scope },
            ReferenceParsingKind::Allow
        ));
        it += typeName.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CloseBracket);
        ++it;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenParen);
        ++it;

        ACE_TRY(expr, ParseExpr({ it, t_context.Scope }));
        it += expr.Length;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::CloseParen);
        ++it;

        return Measured
        {
            std::make_shared<const DerefAsExprNode>(
                typeName.Value,
                expr.Value
            ),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseAttribute(
        const ParseContext& t_context
    ) -> Expected<Measured<std::shared_ptr<const AttributeNode>>>
    {
        auto it = t_context.Iterator;

        ACE_TRY_ASSERT(it->Unwrap().Kind == TokenKind::OpenBracket);
        ++it;

        ACE_TRY(structConstructionExpr, ParseStructConstructionExpr({ it, t_context.Scope }));
        it += structConstructionExpr.Length;

        return Measured
        {
            std::make_shared<const AttributeNode>(structConstructionExpr.Value),
            Distance(t_context.Iterator, it),
        };
    }

    auto Parser::ParseAttributes(
        const ParseContext& t_context
    ) -> Expected<Measured<std::vector<std::shared_ptr<const AttributeNode>>>>
    {
        auto it = t_context.Iterator;

        std::vector<std::shared_ptr<const AttributeNode>> attributes{};
        while (const auto expAttribute = ParseAttribute({ it, t_context.Scope }))
        {
            attributes.push_back(expAttribute.Unwrap().Value);
            it += expAttribute.Unwrap().Length;
        }

        return Measured
        {
            attributes,
            Distance(t_context.Iterator, it),
        };
    }
}
