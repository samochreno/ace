#include "Scope.hpp"

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <map>

#include "Asserts.hpp"
#include "Error.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/All.hpp"
#include "SymbolCreatable.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression/ConversionPlaceholder.hpp"
#include "Compilation.hpp"

namespace Ace
{
    static auto AreTypesSame(
        const std::vector<Symbol::Type::IBase*>& t_typesA,
        const std::vector<Symbol::Type::IBase*>& t_typesB
    ) -> bool
    {
        if (t_typesA.size() != t_typesB.size())
            return false;

        for (size_t i = 0; i < t_typesA.size(); i++)
        {
            if (
                t_typesA.at(i)->GetUnaliased() !=
                t_typesB.at(i)->GetUnaliased()
                )
                return false;
        }

        return true;
    }

     auto FindTemplatedImplContext(
        const std::shared_ptr<const Scope>& t_scope
    ) -> std::optional<Symbol::TemplatedImpl*>
    {
        std::shared_ptr<const Scope> scope = t_scope;
        for (
            ; 
            scope->GetParent().has_value(); 
            scope = scope->GetParent().value()
            )
        {
            const auto parentScope = scope->GetParent().value();

            const auto templatedImplSymbols = parentScope->CollectSymbols<Symbol::TemplatedImpl>();
            const auto foundIt = std::find_if(
                begin(templatedImplSymbols),
                end  (templatedImplSymbols),
                [&](Symbol::TemplatedImpl* const t_templatedImpl)
                {
                    return t_templatedImpl->GetSelfScope().get() == scope.get();
                }
            );

            if (foundIt == end(templatedImplSymbols))
                continue;

            return *foundIt;
        }

        return nullptr;
    }

    static auto IsSameTemplatedImplContext(
        const std::shared_ptr<const Scope>& t_scopeA, 
        const std::shared_ptr<const Scope>& t_scopeB
    ) -> bool
    {
        const auto contextA = FindTemplatedImplContext(t_scopeA).value();
        const auto contextB = FindTemplatedImplContext(t_scopeB).value();

        return
            contextA->GetImplementedTypeTemplate() == 
            contextB->GetImplementedTypeTemplate();
    }

    static auto ResolveTemplateArguments(
        const std::shared_ptr<const Scope>& t_scope,
        const std::vector<SymbolName>& t_templateArgumentNames
    ) -> Expected<std::vector<Symbol::Type::IBase*>>
    {
        ACE_TRY(templateArguments, TransformExpectedVector(t_templateArgumentNames,
        [&](const SymbolName& t_typeName)
        {
            return t_scope->ResolveStaticSymbol<Symbol::Type::IBase>(t_typeName);
        }));

        return templateArguments;
    }

    static auto IsInstanceVariable(
        const SymbolResolutionData& t_data,
        const std::vector<std::unique_ptr<Symbol::IBase>>& t_symbols
    ) -> bool
    {
        if (!t_data.IsLastNameSection)
            return false;

        if (t_symbols.size() != 1)
            return false;

        auto* const symbol = t_symbols.front().get();

        auto* const instanceVariableSymbol =
            dynamic_cast<Symbol::Variable::Normal::Instance*>(symbol);

        if (!instanceVariableSymbol)
            return false;

        return true;
    }

    auto IsSymbolVisibleFromScope(
        Symbol::IBase* const t_symbol,
        const std::shared_ptr<const Scope>& t_scope
    ) -> bool
    {
        switch (t_symbol->GetAccessModifier())
        {
            case AccessModifier::Public:
            {
                return true;
            }

            case AccessModifier::Private:
            {
                const auto optSymbolModule = t_symbol->GetScope()->FindModule();
                if (!optSymbolModule.has_value())
                    return true; 

                auto* const symbolModule = optSymbolModule.value();

                const auto optScopeModule = t_scope->FindModule();
                if (!optScopeModule.has_value())
                    return false;

                auto* const scopeModule = optScopeModule.value();

                if (symbolModule == scopeModule)
                    return true;

                const bool isSymbolChildOfScope = symbolModule->GetSelfScope()->HasChild(
                    scopeModule->GetSelfScope()
                );
                if (isSymbolChildOfScope)
                    return true;

                return false;
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    auto GetSymbolCategory(Symbol::IBase* const t_symbol) -> SymbolCategory
    {
        return t_symbol->GetSymbolCategory();
    }

    SymbolResolutionData::SymbolResolutionData(
        const std::shared_ptr<const Scope>& t_resolvingFromScope,
        const std::vector<SymbolNameSection>::const_iterator& t_nameSectionsBegin,
        const std::vector<SymbolNameSection>::const_iterator& t_nameSectionsEnd,
        const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes,
        const std::function<bool(const Symbol::IBase* const)>& t_isCorrectSymbolType,
        const std::vector<std::shared_ptr<const Scope>>& t_scopes,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const bool t_isSymbolTemplate
    ) : ResolvingFromScope{ t_resolvingFromScope },
        NameSectionsBegin{ t_nameSectionsBegin },
        NameSectionsEnd{ t_nameSectionsEnd },
        OptArgumentTypes{ t_optArgumentTypes },
        IsCorrectSymbolType{ t_isCorrectSymbolType },
        Scopes{ t_scopes },
        ImplTemplateArguments{ t_implTemplateArguments },
        IsSymbolTemplate{ t_isSymbolTemplate },
        IsLastNameSection{ std::distance(t_nameSectionsBegin, t_nameSectionsEnd) == 1 },
        Name{ t_nameSectionsBegin->Name },
        TemplateName{ SpecialIdentifier::CreateTemplate(Name) }
    {
    }

    Scope::Scope(
        const Compilation& t_compilation
    ) : Scope
        {
            t_compilation,
            std::string{ SpecialIdentifier::Global },
            std::nullopt
        }
    {
    }
    
    static auto FindExpiredChild(
        const std::vector<std::weak_ptr<Scope>>& t_children
    ) -> std::vector<std::weak_ptr<Scope>>::const_iterator
    {
        return std::find_if(
            begin(t_children),
            end  (t_children),
            [&](const std::weak_ptr<Scope>& t_child)
            {
                return t_child.expired();
            }
        );
    }

    Scope::~Scope()
    {
        if (!m_OptParent.has_value())
            return;

        auto& parentChildren = m_OptParent.value()->m_Children;

        const auto foundIt = FindExpiredChild(parentChildren);
        ACE_ASSERT(foundIt != end(parentChildren));
        parentChildren.erase(foundIt);

        ACE_ASSERT(FindExpiredChild(parentChildren) == end(parentChildren));
    }

    auto Scope::FindModule() const -> std::optional<Symbol::Module*>
    {
        auto child = shared_from_this();
        auto optParent = child->GetParent();

        const auto moveUp = [&]() -> void
        {
            child = optParent.value(); 
            optParent = optParent.value()->GetParent();
        };

        for (; optParent.has_value(); moveUp())
        {
            const auto parent = optParent.value();

            const auto foundIt = parent->m_SymbolMap.find(child->GetName());

            if (foundIt == end(parent->m_SymbolMap))
                continue;

            if (foundIt->second.size() != 1)
                continue;

            auto* const moduleSymbol = dynamic_cast<Symbol::Module*>(
                foundIt->second.front().get()
            );
            if (!moduleSymbol)
                continue;

            return moduleSymbol;
        }

        return std::nullopt;
    }

    auto Scope::GetOrCreateChild(
        const std::optional<std::string>& t_optName
    ) -> std::shared_ptr<Scope>
    {
        if (t_optName)
        {
            const auto foundIt = std::find_if(
                begin(m_Children),
                end  (m_Children),
                [&](const std::weak_ptr<Scope>& t_child)
                {
                    return t_child.lock()->m_Name == t_optName.value();
                }
            );

            if (foundIt != end(m_Children))
            {
                return foundIt->lock();
            }
        }

        return AddChild(t_optName);
    }

    auto Scope::HasChild(const std::shared_ptr<const Scope>& t_scope) const -> bool
    {
        const auto foundIt = std::find_if(
            begin(m_Children),
            end  (m_Children),
            [&](const std::weak_ptr<Scope>& t_child)
            {
                const auto child = t_child.lock();

                if (child.get() == t_scope.get())
                    return true;

                if (child->HasChild(t_scope))
                    return true;

                return false;
            }
        );

        return foundIt != end(m_Children);
    }

    auto Scope::DefineSymbol(
        const ISymbolCreatable* const t_creatable
    ) -> Expected<Symbol::IBase*>
    {
        const auto scope = t_creatable->GetSymbolScope();

        if (auto* const partiallyCreatable = dynamic_cast<const IPartiallySymbolCreatable*>(t_creatable))
        {
            const auto optDefinedSymbol = scope->GetDefinedSymbol(
                partiallyCreatable->GetName(),
                {},
                {}
            );

            if (optDefinedSymbol.has_value())
            {
                ACE_TRY_VOID(partiallyCreatable->ContinueCreatingSymbol(
                    optDefinedSymbol.value()
                ));

                return optDefinedSymbol.value();
            }
        }

        ACE_TRY(symbol, t_creatable->CreateSymbol());
        return DefineSymbol(std::move(symbol));
    }

    auto Scope::RemoveSymbol(Symbol::IBase* const t_symbol) -> void
    {
        const auto scope = t_symbol->GetScope();
        auto& symbols = scope->m_SymbolMap.at(t_symbol->GetName());

        const auto foundIt = std::find_if(begin(symbols), end(symbols),
        [&](const std::unique_ptr<Symbol::IBase>& t_ownedSymbol)
        {
            return t_ownedSymbol.get() == t_symbol;
        });
        ACE_ASSERT(foundIt != end(symbols));

        symbols.erase(foundIt);
    }

    auto Scope::DefineAssociation(const std::shared_ptr<Scope>& t_association) -> void
    {
        m_Associations.insert(t_association);
    }

    auto Scope::CreateArgumentTypes(
        Symbol::Type::IBase* const t_argumentType
    ) -> std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>
    {
        std::vector<Symbol::Type::IBase*> argumentTypes{ t_argumentType };
        std::reference_wrapper<const std::vector<Symbol::Type::IBase*>> argumentTypesRef{ argumentTypes };
        return argumentTypesRef;
    }
    auto Scope::CreateArgumentTypes(
        const std::vector<Symbol::Type::IBase*>& t_argumentTypes
    ) -> std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>
    {
        std::reference_wrapper<const std::vector<Symbol::Type::IBase*>> argumentTypesRef{ t_argumentTypes };
        return argumentTypesRef;
    }

    auto Scope::CollectAllDefinedSymbols() const -> std::vector<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};

        std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
        [&](const std::pair<const std::string&, const std::vector<std::unique_ptr<Symbol::IBase>>&>& t_pair)
        {
            std::transform(begin(t_pair.second), end(t_pair.second), back_inserter(symbols),
            [](const std::unique_ptr<Symbol::IBase>& t_symbol)
            {
                return t_symbol.get();
            });
        });

        return symbols;
    }

    auto Scope::CollectAllDefinedSymbolsRecursive() const -> std::vector<Symbol::IBase*>
    {
        auto symbols = CollectAllDefinedSymbols();

        std::for_each(begin(m_Children), end(m_Children),
        [&](const std::weak_ptr<Scope>& t_child)
        {
            auto childSymbols = t_child.lock()->CollectAllDefinedSymbolsRecursive();
            symbols.insert(
                end(symbols),
                begin(childSymbols),
                end  (childSymbols)
            );
        });

        return symbols;
    }

    struct TemplateArgumentDeductionResult
    {
        Symbol::Type::TemplateParameter::Normal* TemplateParameter{};
        Symbol::Type::IBase* TemplateArgument{};
    };

    static auto DeduceTemplateArguments(
        Symbol::Type::IBase* t_argumentType,
        Symbol::Type::IBase* t_parameterType
    ) -> Expected<std::vector<TemplateArgumentDeductionResult>>
    {
        t_argumentType = t_argumentType->GetWithoutReference();
        t_parameterType = t_parameterType->GetWithoutReference();

        auto* const templateParameter = dynamic_cast<Symbol::Type::TemplateParameter::Normal*>(
            t_parameterType->GetUnaliased()
        );
        if (templateParameter)
        {
            return std::vector
            {
                TemplateArgumentDeductionResult
                {
                    templateParameter,
                    t_argumentType,
                }
            };
        }

        const auto optArgumentTypeTemplate = t_argumentType->GetTemplate();
        const auto optParameterTypeTemplate = t_parameterType->GetTemplate();

        ACE_TRY_ASSERT(
            optArgumentTypeTemplate.has_value() ==
            optParameterTypeTemplate.has_value()
        );

        const bool isTemplate = optArgumentTypeTemplate.has_value();
        if (!isTemplate)
            return {};

        const auto argumentTypeTemplateParameters =
            t_argumentType->CollectTemplateArguments();

        const auto parameterTypeTemplateParameters =
            t_parameterType->CollectTemplateArguments();

        std::vector<TemplateArgumentDeductionResult> finalDeductionResults{};
        const auto parametersSize = argumentTypeTemplateParameters.size();
        for (size_t i = 0; i < parametersSize; i++)
        {
            auto* const argumentTypeTemplateParameter =
                argumentTypeTemplateParameters.at(i);

            auto* const parameterTypeTemplateParameter =
                parameterTypeTemplateParameters.at(i);

            ACE_TRY(deductionResults, DeduceTemplateArguments(
                argumentTypeTemplateParameter,
                parameterTypeTemplateParameter
            ));
            
            finalDeductionResults.insert(
                end(finalDeductionResults),
                begin(deductionResults),
                end  (deductionResults)
            );
        }

        return finalDeductionResults;
    }

    static auto DeduceAlgorithm(
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
        const std::vector<Symbol::Type::TemplateParameter::Normal*>& t_templateParameters,
        const std::vector<Symbol::Type::IBase*>& t_argumentTypes,
        const std::vector<Symbol::Type::IBase*>& t_parameterTypes
    ) -> Expected<std::vector<Symbol::Type::IBase*>>
    {
        std::map<Symbol::Type::TemplateParameter::Normal*, Symbol::Type::IBase*> templateArgumentMap{};

        for (size_t i = 0; i < t_templateArguments.size(); i++)
        {
            templateArgumentMap[t_templateParameters.at(i)] = t_templateArguments.at(i);
        }

        for (size_t i = 0; i < t_argumentTypes.size(); i++)
        {
            ACE_TRY(deductionResults, DeduceTemplateArguments(
                t_argumentTypes.at(i),
                t_parameterTypes.at(i)
            ));

            ACE_TRY_VOID(TransformExpectedVector(deductionResults,
            [&](const TemplateArgumentDeductionResult& t_deductionResult) -> Expected<void>
            {
                const auto foundIt = templateArgumentMap.find(
                    t_deductionResult.TemplateParameter
                );

                if (foundIt != end(templateArgumentMap))
                {
                    const bool isAlreadyDefinedSame =
                        foundIt->second == t_deductionResult.TemplateArgument;
                    ACE_TRY_ASSERT(isAlreadyDefinedSame);
                    return ExpectedVoid;
                }

                templateArgumentMap[t_deductionResult.TemplateParameter] =
                    t_deductionResult.TemplateArgument;

                return ExpectedVoid;
            }));
        }

        ACE_TRY(templateArguments, TransformExpectedVector(t_templateParameters,
        [&](Symbol::Type::TemplateParameter::Normal* const t_templateParameter) -> Expected<Symbol::Type::IBase*>
        {
            const auto foundIt = templateArgumentMap.find(t_templateParameter);
            ACE_TRY_ASSERT(foundIt != end(templateArgumentMap));
            return foundIt->second;
        }));

        return templateArguments;
    }

    static auto DeduceTemplateArguments(
        Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
        const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes
    ) -> Expected<std::vector<Symbol::Type::IBase*>>
    {
        const auto templateParameters = t_template->CollectParameters();
        if (templateParameters.size() == t_templateArguments.size())
        {
            return t_templateArguments;
        }

        ACE_TRY_ASSERT(t_optArgumentTypes.has_value());

        auto* const parametrized = dynamic_cast<Symbol::IParameterized*>(
            t_template->GetPlaceholderSymbol()
        );
        ACE_TRY_ASSERT(parametrized);

        const auto parameterTypes = parametrized->CollectParameterTypes();
        ACE_TRY_ASSERT(!parameterTypes.empty());

        return DeduceAlgorithm(
            t_templateArguments,
            templateParameters,
            t_optArgumentTypes.value(),
            parameterTypes
        );
    }
    
    auto Scope::ResolveOrInstantiateTemplateInstance(
        const Compilation& t_compilation,
        Symbol::Template::IBase* const t_template,
        const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments
    ) -> Expected<Symbol::IBase*>
    {
        ACE_TRY(deducedTemplateArguments, DeduceTemplateArguments(
            t_template,
            t_templateArguments,
            t_optArgumentTypes
        ));

        const auto expResolvedInstance = ResolveTemplateInstance(
            t_template,
            t_implTemplateArguments,
            deducedTemplateArguments
        );
        if (expResolvedInstance)
        {
            return expResolvedInstance.Unwrap();
        }

        ACE_TRY(symbol, t_compilation.TemplateInstantiator->InstantiateSymbols(
            t_template,
            t_implTemplateArguments,
            deducedTemplateArguments
        ));

        return symbol;
    }

    auto Scope::CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        auto aliases = CollectSymbols<Symbol::Type::Alias::TemplateArgument::Normal>();
        std::sort(begin(aliases), end(aliases),
        [](
            const Symbol::Type::Alias::TemplateArgument::Normal* const t_lhs,
            const Symbol::Type::Alias::TemplateArgument::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Type::IBase*> arguments{};
        std::transform(
            begin(aliases),
            end  (aliases),
            back_inserter(arguments),
            [](const Symbol::Type::Alias::TemplateArgument::Normal* const t_alias)
            {
                return t_alias->GetAliasedType();
            }
        );

        return arguments;
    }

    auto Scope::CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        auto aliases = CollectSymbols<Symbol::Type::Alias::TemplateArgument::Impl>();
        std::sort(begin(aliases), end(aliases),
        [](
            const Symbol::Type::Alias::TemplateArgument::Impl* const t_lhs,
            const Symbol::Type::Alias::TemplateArgument::Impl* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Type::IBase*> arguments{};
        std::transform(
            begin(aliases),
            end  (aliases),
            back_inserter(arguments),
            [](const Symbol::Type::Alias::TemplateArgument::Impl* const t_alias)
            {
                return t_alias->GetAliasedType();
            }
        );

        return arguments;
    }

    auto Scope::DefineTemplateArgumentAliases(
        const std::vector<std::string>& t_implTemplateParameterNames, 
        const std::vector<Symbol::Type::IBase*> t_implTemplateArguments, 
        const std::vector<std::string>& t_templateParameterNames, 
        const std::vector<Symbol::Type::IBase*> t_templateArguments
    ) -> Expected<void>
    {
        ACE_TRY_ASSERT(
            t_implTemplateParameterNames.size() ==
            t_implTemplateArguments.size()
        );
        ACE_TRY_ASSERT(
            t_templateParameterNames.size() ==
            t_templateArguments.size()
        );

        for (size_t i = 0; i < t_implTemplateParameterNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<Symbol::Type::Alias::TemplateArgument::Impl>(
                shared_from_this(),
                t_implTemplateParameterNames.at(i),
                t_implTemplateArguments.at(i),
                i
                );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        for (size_t i = 0; i < t_templateParameterNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<Symbol::Type::Alias::TemplateArgument::Normal>(
                shared_from_this(),
                t_templateParameterNames.at(i),
                t_templateArguments.at(i),
                i
                );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        return ExpectedVoid;
    }

    Scope::Scope(
        const Compilation& t_compilation,
        const std::optional<std::string>& t_optName,
        const std::optional<std::shared_ptr<Scope>>& t_optParent
    ) : m_Compilation{ t_compilation },
        m_OptParent{ t_optParent },
        m_SymbolMap{}
    {
        m_NestLevel = t_optParent.has_value() ?
            (t_optParent.value()->GetNestLevel() + 1) :
            0;

        m_Name = t_optName.has_value() ?
            t_optName.value() :
            SpecialIdentifier::CreateAnonymous();
    }

    auto Scope::AddChild(const std::optional<std::string>& t_optName) -> std::shared_ptr<Scope>
    {
        ChildCount++;

        std::shared_ptr<Scope> child
        {
            new Scope(
                m_Compilation,
                t_optName,
                shared_from_this()
            )
        };

        m_Children.push_back(child);
        return child;
    }

    auto Scope::CanDefineSymbol(const Symbol::IBase* const t_symbol) -> bool
    {
        // TODO: Dont allow private types to leak in public interface.

#if 0 // This doesnt work for associated functions parameters, needs rework.
        if (auto typedSymbol = dynamic_cast<const Symbol::IBase*>(t_symbol))
        {
            if (
                (typedSymbol->GetType()->GetAccessModifier() == AccessModifier::Private) && 
                (typedSymbol->GetAccessModifier() != AccessModifier::Private)
                )
                return false;
        }
#endif

        const auto [templateArguments, implTemplateArguments] = [&]() -> std::tuple<std::vector<Symbol::Type::IBase*>, std::vector<Symbol::Type::IBase*>>
        {
            const auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(
                t_symbol
            );
            if (!templatableSymbol)
                return {};

            return 
            { 
                templatableSymbol->CollectTemplateArguments(), 
                templatableSymbol->CollectImplTemplateArguments() 
            };
        }();

        return !GetDefinedSymbol(
            t_symbol->GetName(), 

            templateArguments, 
            implTemplateArguments
        ).has_value();
    }

    auto Scope::GetDefinedSymbol(
        const std::string& t_name,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments
    ) -> std::optional<Symbol::IBase*>
    {
        const auto foundIt = m_SymbolMap.find(t_name);
        ACE_TRY_ASSERT(foundIt != end(m_SymbolMap));

        const bool isTemplateInstance = 
            !t_templateArguments.empty() || 
            !t_implTemplateArguments.empty();

        if (isTemplateInstance)
        {
            const auto perfectMatchIt = std::find_if(
                begin(foundIt->second), 
                end  (foundIt->second), 
                [&] (const std::unique_ptr<Symbol::IBase>& t_symbol)
                {
                    auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(
                        t_symbol.get()
                    );
                    ACE_ASSERT(templatableSymbol);

                    const auto templateArguments =
                        templatableSymbol->CollectTemplateArguments();

                    const bool doTemplateArgumentsMatch = AreTypesSame(
                        templateArguments,
                        t_templateArguments
                    );

                    if (!doTemplateArgumentsMatch)
                        return false;

                    const auto implTemplateArguments =
                        templatableSymbol->CollectImplTemplateArguments();

                    const bool doImplTemplateArgumentsMatch = AreTypesSame(
                        implTemplateArguments,
                        t_implTemplateArguments
                    );

                    if (!doImplTemplateArgumentsMatch)
                        return false;

                    return true;
                }
            );

            return (perfectMatchIt == end(foundIt->second)) ?
                std::optional<Symbol::IBase*>{} :
                perfectMatchIt->get();
        }
        else
        {
            ACE_ASSERT(foundIt->second.size() == 1);
            return foundIt->second.front().get();
        }
    }

    static auto ResolveLastNameSectionSymbolFromCollectedSymbols(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::IBase*>& t_collectedSymbols
    ) -> Expected<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};
        std::copy_if(
            begin(t_collectedSymbols), 
            end  (t_collectedSymbols), 
            back_inserter(symbols), 
            [&](Symbol::IBase* const t_collectedSymbol)
            {
                return t_data.IsCorrectSymbolType(t_collectedSymbol);
            }
        );

        ACE_TRY_ASSERT(!symbols.empty());
        ACE_ASSERT(symbols.size() == 1);

        auto* const symbol = symbols.front();
        ACE_TRY_ASSERT(IsSymbolVisibleFromScope(
            symbol,
            t_data.ResolvingFromScope
        ));

        return symbol;
    }

    auto Scope::ResolveSymbolInScopes(
        const SymbolResolutionData& t_data
    ) -> Expected<Symbol::IBase*>
    {
        ACE_TRY(templateArguments, ResolveTemplateArguments(
            t_data.ResolvingFromScope,
            t_data.NameSectionsBegin->TemplateArguments
        ));

        return ResolveSymbolInScopes(t_data, templateArguments);
    }

    auto Scope::ResolveSymbolInScopes(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments
    ) -> Expected<Symbol::IBase*>
    {
        ACE_ASSERT(
            t_data.NameSectionsBegin->TemplateArguments.empty() || 
            (
                t_templateArguments.size() == 
                t_data.NameSectionsBegin->TemplateArguments.size()
            )
        );

        std::vector<Symbol::IBase*> symbols{};
        std::for_each(begin(t_data.Scopes), end(t_data.Scopes),
        [&](const std::shared_ptr<const Scope>& t_scope)
        {
            const auto matchingSymbols = 
                t_scope->CollectMatchingSymbols(t_data, t_templateArguments);

            symbols.insert(
                end(symbols),
                begin(matchingSymbols),
                end  (matchingSymbols)
            );
        });

        if (t_data.IsLastNameSection)
        {
            return ResolveLastNameSectionSymbolFromCollectedSymbols(
                t_data,
                symbols
            );
        }
        else
        {
            ACE_TRY_ASSERT(symbols.size() == 1);

            auto* const selfScopedSymbol = dynamic_cast<Symbol::ISelfScoped*>(
                symbols.front()
            );
            ACE_TRY_ASSERT(selfScopedSymbol);

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(
                selfScopedSymbol, 
                t_data.ResolvingFromScope
            ));

            std::vector<std::shared_ptr<const Scope>> scopes{};
            scopes.push_back(selfScopedSymbol->GetSelfScope());
            const auto& selfScopeAssociations = selfScopedSymbol->GetSelfScope()->m_Associations;
            scopes.insert(
                end(scopes),
                begin(selfScopeAssociations),
                end  (selfScopeAssociations)
            );

            return ResolveSymbolInScopes(SymbolResolutionData{
                t_data.ResolvingFromScope,
                t_data.NameSectionsBegin + 1,
                t_data.NameSectionsEnd,
                t_data.OptArgumentTypes,
                t_data.IsCorrectSymbolType,
                scopes,
                t_templateArguments,
                t_data.IsSymbolTemplate
            });
        }
    }

    auto Scope::CollectMatchingSymbols(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments
    ) const -> std::vector<Symbol::IBase*>
    {
        const auto remainingNameSectionsSize = std::distance(
            t_data.NameSectionsBegin,
            t_data.NameSectionsEnd
        );
        const bool isLastNameSection = remainingNameSectionsSize == 1;

        const auto foundTemplateIt = m_SymbolMap.find(t_data.TemplateName);
        const bool isTemplate = foundTemplateIt != end(m_SymbolMap);

        const auto foundIt = isTemplate ? 
            foundTemplateIt :
            m_SymbolMap.find(t_data.Name);

        if (foundIt == end(m_SymbolMap))
            return {};

        const auto& nameMatchedSymbols = foundIt->second;

        const bool isInstanceVariable = IsInstanceVariable(
            t_data,
            nameMatchedSymbols
        );

        const bool isTemplateSymbol = 
            isTemplate && isLastNameSection && !isInstanceVariable;

        if (isTemplateSymbol)
        {
            return CollectMatchingTemplateSymbols(
                t_data,
                t_templateArguments,
                nameMatchedSymbols
            );
        }

        return CollectMatchingNormalSymbols(
            t_data,
            t_templateArguments,
            nameMatchedSymbols
        );
    }

    auto Scope::CollectMatchingNormalSymbols(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
        const std::vector<std::unique_ptr<Symbol::IBase>>& t_nameMatchedSymbols
    ) const -> std::vector<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};
        std::transform(
            begin(t_nameMatchedSymbols), 
            end  (t_nameMatchedSymbols), 
            back_inserter(symbols), 
            [&](const std::unique_ptr<Symbol::IBase>& t_symbol)
            {
                return t_symbol.get();
            }
        );

        return symbols;
    }

    auto Scope::CollectMatchingTemplateSymbols(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
        const std::vector<std::unique_ptr<Symbol::IBase>>& t_nameMatchedSymbols
    ) const -> std::vector<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};

        ACE_ASSERT(t_nameMatchedSymbols.size() == 1);
        auto* const tmplate = dynamic_cast<Symbol::Template::IBase*>(
            t_nameMatchedSymbols.front().get()
        );
        ACE_ASSERT(tmplate);

        if (t_data.IsSymbolTemplate)
        {
            symbols.push_back(tmplate);
        }
        else
        {
            const auto expTemplateInstance = ResolveOrInstantiateTemplateInstance(
                GetCompilation(),
                tmplate,
                t_data.OptArgumentTypes,
                t_data.ImplTemplateArguments,
                t_templateArguments
            );
            if (!expTemplateInstance)
                return {};
            
            symbols.push_back(expTemplateInstance.Unwrap());
        }

        return symbols;
    }

    auto Scope::ResolveTemplateInstance(
        const Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments
    ) -> Expected<Symbol::IBase*>
    {
        const auto scope = t_template->GetScope();

        const auto foundIt = scope->m_SymbolMap.find(t_template->GetASTName());
        ACE_TRY_ASSERT(foundIt != end(scope->m_SymbolMap));

        auto& symbols = foundIt->second;

        const auto perfectCandidateIt = std::find_if(begin(symbols), end(symbols),
        [&](const std::unique_ptr<Symbol::IBase>& t_symbol)
        {
            auto* const templatableSymbol = dynamic_cast<Symbol::ITemplatable*>(
                t_symbol.get()
            );
            ACE_ASSERT(templatableSymbol);

            const auto collectedTemplateArguments =
                templatableSymbol->CollectTemplateArguments();

            const bool doTemplateArgumentsMatch = AreTypesSame(
                t_templateArguments,
                collectedTemplateArguments
            );

            if (!doTemplateArgumentsMatch)
                return false;

            const auto collectedImplTemplateArguments =
                templatableSymbol->CollectImplTemplateArguments();

            if (!collectedImplTemplateArguments.empty())
            {
                const bool doImplTemplateArgumentsMatch = AreTypesSame(
                    t_implTemplateArguments,
                    collectedImplTemplateArguments
                );

                if (!doImplTemplateArgumentsMatch)
                    return false;
            }
            
            return true;
        });

        ACE_TRY_ASSERT(perfectCandidateIt != end(symbols));
        return perfectCandidateIt->get();
    }

    auto Scope::ExclusiveResolveTemplate(
        const std::string& t_name
    ) const -> Expected<Symbol::Template::IBase*>
    {
        auto foundIt = m_SymbolMap.find(t_name);
        ACE_TRY_ASSERT(foundIt != end(m_SymbolMap));

        std::vector<Symbol::IBase*> symbols{};
        std::transform(
            begin(foundIt->second),
            end  (foundIt->second),
            back_inserter(symbols),
            [](const std::unique_ptr<Symbol::IBase>& t_symbol)
            {
                return t_symbol.get();
            }
        );

        ACE_TRY_ASSERT(symbols.size() == 1);

        auto* const castedSymbol = dynamic_cast<Symbol::Template::IBase*>(
            symbols.front()
        );
        ACE_ASSERT(castedSymbol);

        return castedSymbol;
    }

    auto Scope::GetInstanceSymbolResolutionScopes(
        Symbol::Type::IBase* t_selfType
    ) -> std::vector<std::shared_ptr<const Scope>>
    {
        t_selfType = t_selfType->GetUnaliased();

        if (t_selfType->IsReference())
        {
            return GetInstanceSymbolResolutionScopes(
                t_selfType->GetWithoutReference()
            );
        }

        if (t_selfType->IsStrongPointer())
        {
            return GetInstanceSymbolResolutionScopes(
                t_selfType->GetWithoutStrongPointer()
            );
        }

        const auto typeSelfScope = t_selfType->GetSelfScope();
        
        std::vector<std::shared_ptr<const Scope>> scopes{};
        scopes.push_back(typeSelfScope);

        const auto optTemplate = t_selfType->GetTemplate();
        const auto associationsScope = optTemplate.has_value() ?
            optTemplate.value()->GetSelfScope() :
            typeSelfScope;

        scopes.insert(
            end(scopes),
            begin(associationsScope->m_Associations),
            end  (associationsScope->m_Associations)
        );

        return scopes;
    }

    auto Scope::CollectInstanceSymbolResolutionImplTemplateArguments(
        Symbol::Type::IBase* const t_selfType
    ) -> std::vector<Symbol::Type::IBase*>
    {
        return t_selfType->CollectTemplateArguments();
    }

    auto Scope::GetStaticSymbolResolutionStartScope(
        const SymbolName& t_name
    ) const -> Expected<std::shared_ptr<const Scope>>
    {
        if (t_name.IsGlobal)
        {
            return std::shared_ptr<const Scope>
            { 
                m_Compilation.GlobalScope
            };
        }

        const auto& section = t_name.Sections.front();

        const auto& name = section.Name;
        const auto templateName = SpecialIdentifier::CreateTemplate(name);

        for (
            auto optScope = std::optional{ shared_from_this() }; 
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            const auto scope = optScope.value();

            if (
                !scope->m_SymbolMap.contains(templateName) &&
                !scope->m_SymbolMap.contains(name)
                )
                continue;

            return scope;
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Scope::GetStaticSymbolResolutionImplTemplateArguments(
        const std::shared_ptr<const Scope>& t_startScope
    ) -> std::vector<Symbol::Type::IBase*>
    {
        for (
            std::optional<std::shared_ptr<const Scope>> optScope = t_startScope;
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            const auto scope = optScope.value();

            const auto implTemplateArguments =
                scope->CollectImplTemplateArguments();

            if (!implTemplateArguments.empty())
            {
                return implTemplateArguments;
            }
        }

        return {};
    }
}
