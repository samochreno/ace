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

#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbols/All.hpp"
#include "SymbolCreatable.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"
#include "Compilation.hpp"
#include "TypeConversions.hpp"

namespace Ace
{
    auto FindTemplatedImplContext(
        const std::shared_ptr<const Scope>& startScope
    ) -> std::optional<TemplatedImplSymbol*>
    {
        std::shared_ptr<const Scope> scope = startScope;
        for (
            ; 
            scope->GetParent().has_value(); 
            scope = scope->GetParent().value()
            )
        {
            const auto parentScope = scope->GetParent().value();

            const auto templatedImplSymbols =
                parentScope->CollectSymbols<TemplatedImplSymbol>();

            const auto matchingTemplatedImplIt = std::find_if(
                begin(templatedImplSymbols),
                end  (templatedImplSymbols),
                [&](TemplatedImplSymbol* const templatedImpl)
                {
                    return templatedImpl->GetSelfScope().get() == scope.get();
                }
            );

            if (matchingTemplatedImplIt == end(templatedImplSymbols))
            {
                continue;
            }

            return *matchingTemplatedImplIt;
        }

        return nullptr;
    }

    static auto IsSameTemplatedImplContext(
        const std::shared_ptr<const Scope>& scopeA, 
        const std::shared_ptr<const Scope>& scopeB
    ) -> bool
    {
        const auto contextA = FindTemplatedImplContext(scopeA).value();
        const auto contextB = FindTemplatedImplContext(scopeB).value();

        return
            contextA->GetImplementedTypeTemplate() == 
            contextB->GetImplementedTypeTemplate();
    }

    static auto ResolveTemplateArgs(
        const std::shared_ptr<const Scope>& scope,
        const std::vector<SymbolName>& templateArgNames
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        ACE_TRY(templateArgs, TransformExpectedVector(templateArgNames,
        [&](const SymbolName& typeName)
        {
            return scope->ResolveStaticSymbol<ITypeSymbol>(typeName);
        }));

        return templateArgs;
    }

    static auto IsInstanceVar(
        const SymbolResolutionData& data,
        const std::vector<std::unique_ptr<ISymbol>>& symbols
    ) -> bool
    {
        if (!data.IsLastNameSection)
        {
            return false;
        }

        if (symbols.size() != 1)
        {
            return false;
        }

        auto* const symbol = symbols.front().get();

        auto* const instanceVarSymbol =
            dynamic_cast<InstanceVarSymbol*>(symbol);

        if (!instanceVarSymbol)
        {
            return false;
        }

        return true;
    }

    auto IsSymbolVisibleFromScope(
        ISymbol* const symbol,
        const std::shared_ptr<const Scope>& scope
    ) -> bool
    {
        switch (symbol->GetAccessModifier())
        {
            case AccessModifier::Public:
            {
                return true;
            }

            case AccessModifier::Private:
            {
                const auto optSymbolModule = symbol->GetScope()->FindModule();
                if (!optSymbolModule.has_value())
                {
                    return true; 
                }

                auto* const symbolModule = optSymbolModule.value();

                const auto optScopeModule = scope->FindModule();
                if (!optScopeModule.has_value())
                {
                    return false;
                }

                auto* const scopeModule = optScopeModule.value();

                if (symbolModule == scopeModule)
                {
                    return true;
                }

                const bool isSymbolChildOfScope = symbolModule->GetSelfScope()->HasChild(
                    scopeModule->GetSelfScope()
                );
                if (isSymbolChildOfScope)
                {
                    return true;
                }

                return false;
            }
        }
    }

    auto GetSymbolCategory(ISymbol* const symbol) -> SymbolCategory
    {
        return symbol->GetSymbolCategory();
    }

    SymbolResolutionData::SymbolResolutionData(
        const std::shared_ptr<const Scope>& resolvingFromScope,
        const std::vector<SymbolNameSection>::const_iterator nameSectionsBegin,
        const std::vector<SymbolNameSection>::const_iterator nameSectionsEnd,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
        const std::function<bool(const ISymbol* const)>& isCorrectSymbolType,
        const std::vector<std::shared_ptr<const Scope>>& scopes,
        const std::vector<ITypeSymbol*>& implTemplateArgs,
        const bool isTemplate
    ) : ResolvingFromScope{ resolvingFromScope },
        NameSectionsBegin{ nameSectionsBegin },
        NameSectionsEnd{ nameSectionsEnd },
        OptArgTypes{ optArgTypes },
        IsCorrectSymbolType{ isCorrectSymbolType },
        Scopes{ scopes },
        ImplTemplateArgs{ implTemplateArgs },
        IsTemplate{ isTemplate },
        IsLastNameSection
        {
            std::distance(nameSectionsBegin, nameSectionsEnd) == 1
        },
        Name{ nameSectionsBegin->Name.String },
        TemplateName{ SpecialIdentifier::CreateTemplate(Name) }
    {
    }

    GlobalScope::GlobalScope()
    {
    }

    GlobalScope::GlobalScope(const Compilation* const compilation)
        : m_Scope{ new Scope(compilation) }
    {
    }

    GlobalScope::~GlobalScope()
    {
        if (!m_Scope)
        {
            return;
        }

        m_Scope->Clear();
    }

    auto GlobalScope::Unwrap() const -> const std::shared_ptr<Scope>&
    {
        return m_Scope;
    }
    
    static auto FindExpiredChild(
        const std::vector<std::weak_ptr<Scope>>& children
    ) -> std::vector<std::weak_ptr<Scope>>::const_iterator
    {
        return std::find_if(
            begin(children),
            end  (children),
            [&](const std::weak_ptr<Scope>& child)
            {
                return child.expired();
            }
        );
    }

    Scope::~Scope()
    {
        if (!m_OptParent.has_value())
        {
            return;
        }

        auto& parentChildren = m_OptParent.value()->m_Children;

        const auto expiredChildIt = FindExpiredChild(parentChildren);
        ACE_ASSERT(expiredChildIt != end(parentChildren));
        parentChildren.erase(expiredChildIt);

        ACE_ASSERT(FindExpiredChild(parentChildren) == end(parentChildren));
    }

    auto Scope::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    auto Scope::GetNestLevel() const -> size_t
    {
        return m_NestLevel;
    }

    auto Scope::GetParent() const -> const std::optional<std::shared_ptr<Scope>>&
    {
        return m_OptParent;
    }

    auto Scope::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Scope::FindModule() const -> std::optional<ModuleSymbol*>
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

            const auto matchingNameSymbolsIt =
                parent->m_SymbolMap.find(child->GetName());
            const auto& matchingNameSymbols = matchingNameSymbolsIt->second;

            if (matchingNameSymbolsIt == end(parent->m_SymbolMap))
            {
                continue;
            }

            if (matchingNameSymbols.size() != 1)
            {
                continue;
            }

            auto* const moduleSymbol = dynamic_cast<ModuleSymbol*>(
                matchingNameSymbols.front().get()
            );
            if (!moduleSymbol)
            {
                continue;
            }

            return moduleSymbol;
        }

        return std::nullopt;
    }

    auto Scope::GetOrCreateChild(
        const std::optional<std::string>& optName
    ) -> std::shared_ptr<Scope>
    {
        if (optName)
        {
            const auto matchingNameChildIt = std::find_if(
                begin(m_Children),
                end  (m_Children),
                [&](const std::weak_ptr<Scope>& child)
                {
                    return child.lock()->m_Name == optName.value();
                }
            );

            if (matchingNameChildIt != end(m_Children))
            {
                return matchingNameChildIt->lock();
            }
        }

        return AddChild(optName);
    }

    auto Scope::HasChild(
        const std::shared_ptr<const Scope>& scope
    ) const -> bool
    {
        const auto childOrChildsParentIt = std::find_if(
            begin(m_Children),
            end  (m_Children),
            [&](const std::weak_ptr<Scope>& ownedChild)
            {
                const auto child = ownedChild.lock();

                if (child.get() == scope.get())
                {
                    return true;
                }

                if (child->HasChild(scope))
                {
                    return true;
                }

                return false;
            }
        );

        return childOrChildsParentIt != end(m_Children);
    }

    auto Scope::DefineSymbol(
        const ISymbolCreatable* const creatable
    ) -> Expected<ISymbol*>
    {
        const auto scope = creatable->GetSymbolScope();

        if (auto* const partiallyCreatable = dynamic_cast<const IPartiallySymbolCreatable*>(creatable))
        {
            const auto optDefinedSymbol = scope->GetDefinedSymbol(
                partiallyCreatable->GetName().String,
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

        ACE_TRY(symbol, creatable->CreateSymbol());
        return DefineSymbol(std::move(symbol));
    }

    auto Scope::RemoveSymbol(ISymbol* const symbol) -> void
    {
        const auto scope = symbol->GetScope();
        auto& symbols = scope->m_SymbolMap.at(symbol->GetName().String);

        const auto matchingSymbolIt = std::find_if(
            begin(symbols),
            end  (symbols),
            [&](const std::unique_ptr<ISymbol>& ownedSymbol)
            {
                return ownedSymbol.get() == symbol;
            }
        );
        ACE_ASSERT(matchingSymbolIt != end(symbols));

        symbols.erase(matchingSymbolIt);
    }

    auto Scope::DefineAssociation(
        const std::shared_ptr<Scope>& association
    ) -> void
    {
        m_Associations.insert(association);
    }

    auto Scope::CreateArgTypes(
        ITypeSymbol* const argType
    ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>
    {
        std::vector<ITypeSymbol*> argTypes{ argType };
        std::reference_wrapper<const std::vector<ITypeSymbol*>> argTypesRef
        {
            argTypes
        };
        return argTypesRef;
    }
    auto Scope::CreateArgTypes(
        const std::vector<ITypeSymbol*>& argTypes
    ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>
    {
        std::reference_wrapper<const std::vector<ITypeSymbol*>> argTypesRef
        {
            argTypes
        };
        return argTypesRef;
    }

    auto Scope::CollectAllDefinedSymbols() const -> std::vector<ISymbol*>
    {
        std::vector<ISymbol*> symbols{};

        std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
        [&](const std::pair<const std::string&, const std::vector<std::unique_ptr<ISymbol>>&>& pair)
        {
            std::transform(
                begin(pair.second),
                end  (pair.second),
                back_inserter(symbols),
                [](const std::unique_ptr<ISymbol>& symbol)
                {
                    return symbol.get();
                }
            );
        });

        return symbols;
    }

    auto Scope::CollectAllDefinedSymbolsRecursive() const -> std::vector<ISymbol*>
    {
        auto symbols = CollectAllDefinedSymbols();

        std::for_each(begin(m_Children), end(m_Children),
        [&](const std::weak_ptr<Scope>& child)
        {
            const auto childSymbols =
                child.lock()->CollectAllDefinedSymbolsRecursive();

            symbols.insert(
                end(symbols),
                begin(childSymbols),
                end  (childSymbols)
            );
        });

        return symbols;
    }

    struct TemplateArgDeductionResult
    {
        NormalTemplateParamTypeSymbol* TemplateParam{};
        ITypeSymbol* TemplateArg{};
    };

    static auto DeduceTemplateArgs(
        ITypeSymbol* argType,
        ITypeSymbol* paramType
    ) -> Expected<std::vector<TemplateArgDeductionResult>>
    {
        argType = argType->GetWithoutReference();
        paramType = paramType->GetWithoutReference();

        auto* const templateParam = dynamic_cast<NormalTemplateParamTypeSymbol*>(
            paramType->GetUnaliased()
        );
        if (templateParam)
        {
            return std::vector
            {
                TemplateArgDeductionResult
                {
                    templateParam,
                    argType,
                }
            };
        }

        const auto optArgTypeTemplate = argType->GetTemplate();
        const auto optParamTypeTemplate = paramType->GetTemplate();

        ACE_TRY_ASSERT(
            optArgTypeTemplate.has_value() ==
            optParamTypeTemplate.has_value()
        );

        const bool isTemplate = optArgTypeTemplate.has_value();
        if (!isTemplate)
        {
            return {};
        }

        const auto argTypeTemplateParams = argType->CollectTemplateArgs();
        const auto paramTypeTemplateParams = paramType->CollectTemplateArgs();

        std::vector<TemplateArgDeductionResult> finalDeductionResults{};
        const auto paramsSize = argTypeTemplateParams.size();
        for (size_t i = 0; i < paramsSize; i++)
        {
            auto* const argTypeTemplateParam =
                argTypeTemplateParams.at(i);

            auto* const paramTypeTemplateParam =
                paramTypeTemplateParams.at(i);

            ACE_TRY(deductionResults, DeduceTemplateArgs(
                argTypeTemplateParam,
                paramTypeTemplateParam
            ));
            
            finalDeductionResults.insert(
                end(finalDeductionResults),
                begin(deductionResults),
                end  (deductionResults)
            );
        }

        return finalDeductionResults;
    }

    static auto TemplateArgumentDeducingAlgorithm(
        const std::vector<ITypeSymbol*>& knownTemplateArgs,
        const std::vector<NormalTemplateParamTypeSymbol*>& templateParams,
        const std::vector<ITypeSymbol*>& argTypes,
        const std::vector<ITypeSymbol*>& paramTypes
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        std::map<NormalTemplateParamTypeSymbol*, ITypeSymbol*> templateArgMap{};

        for (size_t i = 0; i < knownTemplateArgs.size(); i++)
        {
            templateArgMap[templateParams.at(i)] = knownTemplateArgs.at(i);
        }

        for (size_t i = 0; i < argTypes.size(); i++)
        {
            ACE_TRY(deductionResults, DeduceTemplateArgs(
                argTypes.at(i),
                paramTypes.at(i)
            ));

            ACE_TRY_VOID(TransformExpectedVector(deductionResults,
            [&](const TemplateArgDeductionResult& deductionResult) -> Expected<void>
            {
                const auto templateArgIt = templateArgMap.find(
                    deductionResult.TemplateParam
                );

                if (templateArgIt != end(templateArgMap))
                {
                    const bool isAlreadyDefinedSame =
                        templateArgIt->second == deductionResult.TemplateArg;
                    ACE_TRY_ASSERT(isAlreadyDefinedSame);
                    return Void{};
                }

                templateArgMap[deductionResult.TemplateParam] =
                    deductionResult.TemplateArg;

                return Void{};
            }));
        }

        ACE_TRY(templateArgs, TransformExpectedVector(templateParams,
        [&](NormalTemplateParamTypeSymbol* const templateParam) -> Expected<ITypeSymbol*>
        {
            const auto matchingTemplateArgIt = templateArgMap.find(templateParam);
            ACE_TRY_ASSERT(matchingTemplateArgIt != end(templateArgMap));
            return matchingTemplateArgIt->second;
        }));

        return templateArgs;
    }

    static auto DeduceTemplateArgs(
        ITemplateSymbol* const t3mplate,
        const std::vector<ITypeSymbol*>& knownTemplateArgs,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        const auto templateParams = t3mplate->CollectParams();
        if (templateParams.size() == knownTemplateArgs.size())
        {
            return knownTemplateArgs;
        }

        ACE_TRY_ASSERT(optArgTypes.has_value());

        auto* const parametrized = dynamic_cast<IParamizedSymbol*>(
            t3mplate->GetPlaceholderSymbol()
        );
        ACE_TRY_ASSERT(parametrized);

        const auto paramTypes = parametrized->CollectParamTypes();
        ACE_TRY_ASSERT(!paramTypes.empty());

        return TemplateArgumentDeducingAlgorithm(
            knownTemplateArgs,
            templateParams,
            optArgTypes.value(),
            paramTypes
        );
    }
    
    auto Scope::ResolveOrInstantiateTemplateInstance(
        const Compilation* const compilation,
        ITemplateSymbol* const t3mplate,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
        const std::vector<ITypeSymbol*>& implTemplateArgs,
        const std::vector<ITypeSymbol*>& templateArgs
    ) -> Expected<ISymbol*>
    {
        ACE_TRY(deducedTemplateArgs, DeduceTemplateArgs(
            t3mplate,
            templateArgs,
            optArgTypes
        ));

        const auto expResolvedInstance = ResolveTemplateInstance(
            t3mplate,
            implTemplateArgs,
            deducedTemplateArgs
        );
        if (expResolvedInstance)
        {
            return expResolvedInstance.Unwrap();
        }

        ACE_TRY(symbol, compilation->TemplateInstantiator->InstantiateSymbols(
            t3mplate,
            implTemplateArgs,
            deducedTemplateArgs
        ));

        return symbol;
    }

    auto Scope::CollectTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        auto aliases =
            CollectSymbols<NormalTemplateArgAliasTypeSymbol>();

        std::sort(begin(aliases), end(aliases),
        [](
            const NormalTemplateArgAliasTypeSymbol* const lhs,
            const NormalTemplateArgAliasTypeSymbol* const rhs
            )
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        std::vector<ITypeSymbol*> args{};
        std::transform(
            begin(aliases),
            end  (aliases),
            back_inserter(args),
            [](const NormalTemplateArgAliasTypeSymbol* const alias)
            {
                return alias->GetAliasedType();
            }
        );

        return args;
    }

    auto Scope::CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>
    {
        auto aliases =
            CollectSymbols<ImplTemplateArgAliasTypeSymbol>();

        std::sort(begin(aliases), end(aliases),
        [](
            const ImplTemplateArgAliasTypeSymbol* const lhs,
            const ImplTemplateArgAliasTypeSymbol* const rhs
            )
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        std::vector<ITypeSymbol*> args{};
        std::transform(
            begin(aliases),
            end  (aliases),
            back_inserter(args),
            [](const ImplTemplateArgAliasTypeSymbol* const alias)
            {
                return alias->GetAliasedType();
            }
        );

        return args;
    }

    auto Scope::DefineTemplateArgAliases(
        const std::vector<Identifier>& implTemplateParamNames, 
        const std::vector<ITypeSymbol*> implTemplateArgs, 
        const std::vector<Identifier>& templateParamNames, 
        const std::vector<ITypeSymbol*> templateArgs
    ) -> Expected<void>
    {
        ACE_TRY_ASSERT(
            implTemplateParamNames.size() ==
            implTemplateArgs.size()
        );
        ACE_TRY_ASSERT(
            templateParamNames.size() ==
            templateArgs.size()
        );

        for (size_t i = 0; i < implTemplateParamNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<ImplTemplateArgAliasTypeSymbol>(
                shared_from_this(),
                implTemplateParamNames.at(i),
                implTemplateArgs.at(i),
                i
            );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        for (size_t i = 0; i < templateParamNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<NormalTemplateArgAliasTypeSymbol>(
                shared_from_this(),
                templateParamNames.at(i),
                templateArgs.at(i),
                i
            );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        return Void{};
    }

    Scope::Scope(
        const Compilation* const compilation
    ) : Scope
        {
            compilation,
            std::string{ SpecialIdentifier::Global },
            std::nullopt
        }
    {
    }

    auto Scope::Clear() -> void
    {
        std::for_each(begin(m_Children), end(m_Children),
        [](const std::weak_ptr<Scope>& child)
        {
            child.lock()->Clear();
        });

        m_SymbolMap.clear();
        m_OptParent = std::nullopt;
    }

    Scope::Scope(
        const Compilation* const compilation,
        const std::optional<std::string>& optName,
        const std::optional<std::shared_ptr<Scope>>& optParent
    ) : m_Compilation{ compilation },
        m_OptParent{ optParent },
        m_SymbolMap{}
    {
        m_NestLevel = optParent.has_value() ?
            (optParent.value()->GetNestLevel() + 1) :
            0;

        m_Name = optName.has_value() ?
            optName.value() :
            SpecialIdentifier::CreateAnonymous();
    }

    auto Scope::AddChild(const std::optional<std::string>& optName) -> std::shared_ptr<Scope>
    {
        std::shared_ptr<Scope> child
        {
            new Scope(
                m_Compilation,
                optName,
                shared_from_this()
            )
        };

        m_Children.push_back(child);
        return child;
    }

    auto Scope::CanDefineSymbol(const ISymbol* const symbol) -> bool
    {
        // TODO: Dont allow private types to leak in public interface.

#if 0 // This doesnt work for associated functions params, needs rework.
        if (auto typedSymbol = dynamic_cast<const ISymbol*>(symbol))
        {
            if (
                (typedSymbol->GetType()->GetAccessModifier() == AccessModifier::Private) && 
                (typedSymbol->GetAccessModifier() != AccessModifier::Private)
                )
                return false;
        }
#endif

        const auto [templateArgs, implTemplateArgs] = [&]() -> std::tuple<std::vector<ITypeSymbol*>, std::vector<ITypeSymbol*>>
        {
            const auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(
                symbol
            );
            if (!templatableSymbol)
            {
                return {};
            }

            return 
            { 
                templatableSymbol->CollectTemplateArgs(), 
                templatableSymbol->CollectImplTemplateArgs() 
            };
        }();

        return !GetDefinedSymbol(
            symbol->GetName().String, 
            templateArgs, 
            implTemplateArgs
        ).has_value();
    }

    auto Scope::GetDefinedSymbol(
        const std::string& name,
        const std::vector<ITypeSymbol*>& templateArgs,
        const std::vector<ITypeSymbol*>& implTemplateArgs
    ) -> std::optional<ISymbol*>
    {
        const auto matchingNameSymbolsIt = m_SymbolMap.find(name);
        if (matchingNameSymbolsIt == end(m_SymbolMap))
        {
            return std::nullopt;
        }

        const auto& matchingNameSymbols = matchingNameSymbolsIt->second;

        const bool isTemplateInstance = 
            !templateArgs.empty() || 
            !implTemplateArgs.empty();

        if (!isTemplateInstance)
        {
            ACE_ASSERT(matchingNameSymbols.size() == 1);
            return matchingNameSymbols.front().get();
        }

        const auto perfectMatchIt = std::find_if(
            begin(matchingNameSymbols), 
            end  (matchingNameSymbols), 
            [&](const std::unique_ptr<ISymbol>& symbol)
            {
                auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(
                    symbol.get()
                );
                ACE_ASSERT(templatableSymbol);

                const bool doTemplateArgsMatch = AreTypesSame(
                    templateArgs,
                    templatableSymbol->CollectTemplateArgs()
                );
                if (!doTemplateArgsMatch)
                {
                    return false;
                }

                const bool doImplTemplateArgsMatch = AreTypesSame(
                    implTemplateArgs,
                    templatableSymbol->CollectImplTemplateArgs()
                );
                if (!doImplTemplateArgsMatch)
                {
                    return false;
                }

                return true;
            }
        );

        if (perfectMatchIt == end(matchingNameSymbols))
        {
            return std::nullopt;
        }

        return perfectMatchIt->get();
    }

    static auto ResolveLastNameSectionSymbolFromCollectedSymbols(
        const SymbolResolutionData& data,
        const std::vector<ISymbol*>& collectedSymbols
    ) -> Expected<ISymbol*>
    {
        std::vector<ISymbol*> symbols{};
        std::copy_if(
            begin(collectedSymbols), 
            end  (collectedSymbols), 
            back_inserter(symbols), 
            [&](ISymbol* const collectedSymbol)
            {
                return data.IsCorrectSymbolType(collectedSymbol);
            }
        );

        ACE_TRY_ASSERT(!symbols.empty());
        ACE_ASSERT(symbols.size() == 1);

        auto* const symbol = symbols.front();
        ACE_TRY_ASSERT(IsSymbolVisibleFromScope(
            symbol,
            data.ResolvingFromScope
        ));

        return symbol;
    }

    auto Scope::ResolveSymbolInScopes(
        const SymbolResolutionData& data
    ) -> Expected<ISymbol*>
    {
        ACE_TRY(templateArgs, ResolveTemplateArgs(
            data.ResolvingFromScope,
            data.NameSectionsBegin->TemplateArgs
        ));

        return ResolveSymbolInScopes(data, templateArgs);
    }

    auto Scope::ResolveSymbolInScopes(
        const SymbolResolutionData& data,
        const std::vector<ITypeSymbol*>& templateArgs
    ) -> Expected<ISymbol*>
    {
        ACE_ASSERT(
            data.NameSectionsBegin->TemplateArgs.empty() || 
            (
                templateArgs.size() == 
                data.NameSectionsBegin->TemplateArgs.size()
            )
        );

        std::vector<ISymbol*> symbols{};
        std::for_each(begin(data.Scopes), end(data.Scopes),
        [&](const std::shared_ptr<const Scope>& scope)
        {
            const auto matchingSymbols = 
                scope->CollectMatchingSymbols(data, templateArgs);

            symbols.insert(
                end(symbols),
                begin(matchingSymbols),
                end  (matchingSymbols)
            );
        });

        if (data.IsLastNameSection)
        {
            return ResolveLastNameSectionSymbolFromCollectedSymbols(
                data,
                symbols
            );
        }
        else
        {
            ACE_TRY_ASSERT(symbols.size() == 1);

            auto* const selfScopedSymbol = dynamic_cast<ISelfScopedSymbol*>(
                symbols.front()
            );
            ACE_TRY_ASSERT(selfScopedSymbol);

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(
                selfScopedSymbol, 
                data.ResolvingFromScope
            ));

            std::vector<std::shared_ptr<const Scope>> scopes{};
            scopes.push_back(selfScopedSymbol->GetSelfScope());
            const auto& selfScopeAssociations =
                selfScopedSymbol->GetSelfScope()->m_Associations;
            scopes.insert(
                end(scopes),
                begin(selfScopeAssociations),
                end  (selfScopeAssociations)
            );

            return ResolveSymbolInScopes(SymbolResolutionData{
                data.ResolvingFromScope,
                data.NameSectionsBegin + 1,
                data.NameSectionsEnd,
                data.OptArgTypes,
                data.IsCorrectSymbolType,
                scopes,
                templateArgs,
                data.IsTemplate
            });
        }
    }

    auto Scope::CollectMatchingSymbols(
        const SymbolResolutionData& data,
        const std::vector<ITypeSymbol*>& templateArgs
    ) const -> std::vector<ISymbol*>
    {
        const auto matchingTemplateNameSymbolsIt =
            m_SymbolMap.find(data.TemplateName);

        const bool isTemplate =
            matchingTemplateNameSymbolsIt != end(m_SymbolMap);

        const auto matchingNameSymbolsIt = isTemplate ? 
            matchingTemplateNameSymbolsIt :
            m_SymbolMap.find(data.Name);

        if (matchingNameSymbolsIt == end(m_SymbolMap))
        {
            return {};
        }

        const auto& matchingNameSymbols = matchingNameSymbolsIt->second;

        const bool isInstanceVar = IsInstanceVar(
            data,
            matchingNameSymbols
        );

        const auto remainingNameSectionsSize = std::distance(
            data.NameSectionsBegin,
            data.NameSectionsEnd
        );
        const bool isLastNameSection = remainingNameSectionsSize == 1;

        const bool isTemplateSymbol = 
            isTemplate && isLastNameSection && !isInstanceVar;

        if (isTemplateSymbol)
        {
            return CollectMatchingTemplateSymbols(
                data,
                templateArgs,
                matchingNameSymbols
            );
        }
        else
        {
            return CollectMatchingNormalSymbols(
                data,
                templateArgs,
                matchingNameSymbols
            );
        }
    }

    auto Scope::CollectMatchingNormalSymbols(
        const SymbolResolutionData& data,
        const std::vector<ITypeSymbol*>& templateArgs,
        const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
    ) const -> std::vector<ISymbol*>
    {
        std::vector<ISymbol*> symbols{};
        std::transform(
            begin(matchingNameSymbols), 
            end  (matchingNameSymbols), 
            back_inserter(symbols), 
            [&](const std::unique_ptr<ISymbol>& symbol)
            {
                return symbol.get();
            }
        );

        return symbols;
    }

    auto Scope::CollectMatchingTemplateSymbols(
        const SymbolResolutionData& data,
        const std::vector<ITypeSymbol*>& templateArgs,
        const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
    ) const -> std::vector<ISymbol*>
    {
        std::vector<ISymbol*> symbols{};

        ACE_ASSERT(matchingNameSymbols.size() == 1);
        auto* const t3mplate = dynamic_cast<ITemplateSymbol*>(
            matchingNameSymbols.front().get()
        );
        ACE_ASSERT(t3mplate);

        if (data.IsTemplate)
        {
            symbols.push_back(t3mplate);
        }
        else
        {
            const auto expTemplateInstance = ResolveOrInstantiateTemplateInstance(
                GetCompilation(),
                t3mplate,
                data.OptArgTypes,
                data.ImplTemplateArgs,
                templateArgs
            );
            if (!expTemplateInstance)
            {
                return {};
            }
            
            symbols.push_back(expTemplateInstance.Unwrap());
        }

        return symbols;
    }

    auto Scope::ResolveTemplateInstance(
        const ITemplateSymbol* const t3mplate,
        const std::vector<ITypeSymbol*>& implTemplateArgs,
        const std::vector<ITypeSymbol*>& templateArgs
    ) -> Expected<ISymbol*>
    {
        const auto scope = t3mplate->GetScope();

        const auto matchingNameSymbolsIt =
            scope->m_SymbolMap.find(t3mplate->GetASTName().String);
        ACE_TRY_ASSERT(matchingNameSymbolsIt != end(scope->m_SymbolMap));

        auto& symbols = matchingNameSymbolsIt->second;

        const auto perfectCandidateIt = std::find_if(begin(symbols), end(symbols),
        [&](const std::unique_ptr<ISymbol>& symbol)
        {
            auto* const templatableSymbol = dynamic_cast<ITemplatableSymbol*>(
                symbol.get()
            );
            ACE_ASSERT(templatableSymbol);

            const auto collectedTemplateArgs =
                templatableSymbol->CollectTemplateArgs();

            const bool doTemplateArgsMatch = AreTypesSame(
                templateArgs,
                collectedTemplateArgs
            );

            if (!doTemplateArgsMatch)
            {
                return false;
            }

            const auto collectedImplTemplateArgs =
                templatableSymbol->CollectImplTemplateArgs();

            if (!collectedImplTemplateArgs.empty())
            {
                const bool doImplTemplateArgsMatch = AreTypesSame(
                    implTemplateArgs,
                    collectedImplTemplateArgs
                );

                if (!doImplTemplateArgsMatch)
                {
                    return false;
                }
            }
            
            return true;
        });

        ACE_TRY_ASSERT(perfectCandidateIt != end(symbols));
        return perfectCandidateIt->get();
    }

    auto Scope::ExclusiveResolveTemplate(
        const std::string& name
    ) const -> Expected<ITemplateSymbol*>
    {
        auto matchingNameSymbolsIt = m_SymbolMap.find(name);
        ACE_TRY_ASSERT(matchingNameSymbolsIt != end(m_SymbolMap));

        std::vector<ISymbol*> symbols{};
        std::transform(
            begin(matchingNameSymbolsIt->second),
            end  (matchingNameSymbolsIt->second),
            back_inserter(symbols),
            [](const std::unique_ptr<ISymbol>& symbol)
            {
                return symbol.get();
            }
        );

        ACE_TRY_ASSERT(symbols.size() == 1);

        auto* const castedSymbol = dynamic_cast<ITemplateSymbol*>(
            symbols.front()
        );
        ACE_ASSERT(castedSymbol);

        return castedSymbol;
    }

    auto Scope::GetInstanceSymbolResolutionScopes(
        ITypeSymbol* selfType
    ) -> std::vector<std::shared_ptr<const Scope>>
    {
        selfType = selfType->GetUnaliased();

        if (selfType->IsReference())
        {
            return GetInstanceSymbolResolutionScopes(
                selfType->GetWithoutReference()
            );
        }

        if (selfType->IsStrongPointer())
        {
            return GetInstanceSymbolResolutionScopes(
                selfType->GetWithoutStrongPointer()
            );
        }

        const auto typeSelfScope = selfType->GetSelfScope();
        
        std::vector<std::shared_ptr<const Scope>> scopes{};
        scopes.push_back(typeSelfScope);

        const auto optTemplate = selfType->GetTemplate();
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

    auto Scope::CollectInstanceSymbolResolutionImplTemplateArgs(
        ITypeSymbol* const selfType
    ) -> std::vector<ITypeSymbol*>
    {
        return selfType->CollectTemplateArgs();
    }

    auto Scope::GetStaticSymbolResolutionStartScope(
        const SymbolName& name
    ) const -> Expected<std::shared_ptr<const Scope>>
    {
        if (name.IsGlobal)
        {
            return std::shared_ptr<const Scope>
            { 
                m_Compilation->GlobalScope.Unwrap()
            };
        }

        const auto& section = name.Sections.front();

        const auto& nameString = section.Name.String;
        const auto templateNameString = SpecialIdentifier::CreateTemplate(
            nameString
        );

        for (
            auto optScope = std::optional{ shared_from_this() }; 
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            const auto scope = optScope.value();

            if (
                !scope->m_SymbolMap.contains(templateNameString) &&
                !scope->m_SymbolMap.contains(nameString)
                )
            {
                continue;
            }

            return scope;
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Scope::GetStaticSymbolResolutionImplTemplateArgs(
        const std::shared_ptr<const Scope>& startScope
    ) -> std::vector<ITypeSymbol*>
    {
        for (
            std::optional<std::shared_ptr<const Scope>> optScope = startScope;
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            const auto scope = optScope.value();

            const auto implTemplateArgs = scope->CollectImplTemplateArgs();
            if (!implTemplateArgs.empty())
            {
                return implTemplateArgs;
            }
        }

        return {};
    }
}
