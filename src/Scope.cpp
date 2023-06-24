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
#include "Diagnostics.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/All.hpp"
#include "SymbolCreatable.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Expr/ConversionPlaceholder.hpp"
#include "Compilation.hpp"
#include "TypeConversions.hpp"

namespace Ace
{
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

            const auto templatedImplSymbols =
                parentScope->CollectSymbols<Symbol::TemplatedImpl>();

            const auto matchingTemplatedImplIt = std::find_if(
                begin(templatedImplSymbols),
                end  (templatedImplSymbols),
                [&](Symbol::TemplatedImpl* const t_templatedImpl)
                {
                    return t_templatedImpl->GetSelfScope().get() == scope.get();
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

    static auto ResolveTemplateArgs(
        const std::shared_ptr<const Scope>& t_scope,
        const std::vector<SymbolName>& t_templateArgNames
    ) -> Expected<std::vector<Symbol::Type::IBase*>>
    {
        ACE_TRY(templateArgs, TransformExpectedVector(t_templateArgNames,
        [&](const SymbolName& t_typeName)
        {
            return t_scope->ResolveStaticSymbol<Symbol::Type::IBase>(t_typeName);
        }));

        return templateArgs;
    }

    static auto IsInstanceVar(
        const SymbolResolutionData& t_data,
        const std::vector<std::unique_ptr<Symbol::IBase>>& t_symbols
    ) -> bool
    {
        if (!t_data.IsLastNameSection)
        {
            return false;
        }

        if (t_symbols.size() != 1)
        {
            return false;
        }

        auto* const symbol = t_symbols.front().get();

        auto* const instanceVarSymbol =
            dynamic_cast<Symbol::Var::Normal::Instance*>(symbol);

        if (!instanceVarSymbol)
        {
            return false;
        }

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
                {
                    return true; 
                }

                auto* const symbolModule = optSymbolModule.value();

                const auto optScopeModule = t_scope->FindModule();
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
        const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgTypes,
        const std::function<bool(const Symbol::IBase* const)>& t_isCorrectSymbolType,
        const std::vector<std::shared_ptr<const Scope>>& t_scopes,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArgs,
        const bool t_isSymbolTemplate
    ) : ResolvingFromScope{ t_resolvingFromScope },
        NameSectionsBegin{ t_nameSectionsBegin },
        NameSectionsEnd{ t_nameSectionsEnd },
        OptArgTypes{ t_optArgTypes },
        IsCorrectSymbolType{ t_isCorrectSymbolType },
        Scopes{ t_scopes },
        ImplTemplateArgs{ t_implTemplateArgs },
        IsSymbolTemplate{ t_isSymbolTemplate },
        IsLastNameSection{ std::distance(t_nameSectionsBegin, t_nameSectionsEnd) == 1 },
        Name{ t_nameSectionsBegin->Name },
        TemplateName{ SpecialIdentifier::CreateTemplate(Name) }
    {
    }

    GlobalScope::GlobalScope()
    {
    }

    GlobalScope::GlobalScope(const Compilation* const t_compilation)
        : m_Scope{ new Scope(t_compilation) }
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
            {
                continue;
            }

            if (foundIt->second.size() != 1)
            {
                continue;
            }

            auto* const moduleSymbol = dynamic_cast<Symbol::Module*>(
                foundIt->second.front().get()
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

    auto Scope::HasChild(
        const std::shared_ptr<const Scope>& t_scope
    ) const -> bool
    {
        const auto foundIt = std::find_if(
            begin(m_Children),
            end  (m_Children),
            [&](const std::weak_ptr<Scope>& t_child)
            {
                const auto child = t_child.lock();

                if (child.get() == t_scope.get())
                {
                    return true;
                }

                if (child->HasChild(t_scope))
                {
                    return true;
                }

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

        const auto matchingSymbolIt = std::find_if(
            begin(symbols),
            end  (symbols),
            [&](const std::unique_ptr<Symbol::IBase>& t_ownedSymbol)
            {
                return t_ownedSymbol.get() == t_symbol;
            }
        );
        ACE_ASSERT(matchingSymbolIt != end(symbols));

        symbols.erase(matchingSymbolIt);
    }

    auto Scope::DefineAssociation(
        const std::shared_ptr<Scope>& t_association
    ) -> void
    {
        m_Associations.insert(t_association);
    }

    auto Scope::CreateArgTypes(
        Symbol::Type::IBase* const t_argType
    ) -> std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>
    {
        std::vector<Symbol::Type::IBase*> argTypes{ t_argType };
        std::reference_wrapper<const std::vector<Symbol::Type::IBase*>> argTypesRef{ argTypes };
        return argTypesRef;
    }
    auto Scope::CreateArgTypes(
        const std::vector<Symbol::Type::IBase*>& t_argTypes
    ) -> std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>
    {
        std::reference_wrapper<const std::vector<Symbol::Type::IBase*>> argTypesRef{ t_argTypes };
        return argTypesRef;
    }

    auto Scope::CollectAllDefinedSymbols() const -> std::vector<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};

        std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
        [&](const std::pair<const std::string&, const std::vector<std::unique_ptr<Symbol::IBase>>&>& t_pair)
        {
            std::transform(
                begin(t_pair.second),
                end  (t_pair.second),
                back_inserter(symbols),
                [](const std::unique_ptr<Symbol::IBase>& t_symbol)
                {
                    return t_symbol.get();
                }
            );
        });

        return symbols;
    }

    auto Scope::CollectAllDefinedSymbolsRecursive() const -> std::vector<Symbol::IBase*>
    {
        auto symbols = CollectAllDefinedSymbols();

        std::for_each(begin(m_Children), end(m_Children),
        [&](const std::weak_ptr<Scope>& t_child)
        {
            const auto childSymbols =
                t_child.lock()->CollectAllDefinedSymbolsRecursive();

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
        Symbol::Type::TemplateParam::Normal* TemplateParam{};
        Symbol::Type::IBase* TemplateArg{};
    };

    static auto DeduceTemplateArgs(
        Symbol::Type::IBase* t_argType,
        Symbol::Type::IBase* t_paramType
    ) -> Expected<std::vector<TemplateArgDeductionResult>>
    {
        t_argType = t_argType->GetWithoutReference();
        t_paramType = t_paramType->GetWithoutReference();

        auto* const templateParam = dynamic_cast<Symbol::Type::TemplateParam::Normal*>(
            t_paramType->GetUnaliased()
        );
        if (templateParam)
        {
            return std::vector
            {
                TemplateArgDeductionResult
                {
                    templateParam,
                    t_argType,
                }
            };
        }

        const auto optArgTypeTemplate = t_argType->GetTemplate();
        const auto optParamTypeTemplate = t_paramType->GetTemplate();

        ACE_TRY_ASSERT(
            optArgTypeTemplate.has_value() ==
            optParamTypeTemplate.has_value()
        );

        const bool isTemplate = optArgTypeTemplate.has_value();
        if (!isTemplate)
            return {};

        const auto argTypeTemplateParams =
            t_argType->CollectTemplateArgs();

        const auto paramTypeTemplateParams =
            t_paramType->CollectTemplateArgs();

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

    static auto DeduceAlgorithm(
        const std::vector<Symbol::Type::IBase*>& t_templateArgs,
        const std::vector<Symbol::Type::TemplateParam::Normal*>& t_templateParams,
        const std::vector<Symbol::Type::IBase*>& t_argTypes,
        const std::vector<Symbol::Type::IBase*>& t_paramTypes
    ) -> Expected<std::vector<Symbol::Type::IBase*>>
    {
        std::map<Symbol::Type::TemplateParam::Normal*, Symbol::Type::IBase*> templateArgMap{};

        for (size_t i = 0; i < t_templateArgs.size(); i++)
        {
            templateArgMap[t_templateParams.at(i)] =
                t_templateArgs.at(i);
        }

        for (size_t i = 0; i < t_argTypes.size(); i++)
        {
            ACE_TRY(deductionResults, DeduceTemplateArgs(
                t_argTypes.at(i),
                t_paramTypes.at(i)
            ));

            ACE_TRY_VOID(TransformExpectedVector(deductionResults,
            [&](const TemplateArgDeductionResult& t_deductionResult) -> Expected<void>
            {
                const auto templateArgIt = templateArgMap.find(
                    t_deductionResult.TemplateParam
                );

                if (templateArgIt != end(templateArgMap))
                {
                    const bool isAlreadyDefinedSame =
                        templateArgIt->second == t_deductionResult.TemplateArg;
                    ACE_TRY_ASSERT(isAlreadyDefinedSame);
                    return Void;
                }

                templateArgMap[t_deductionResult.TemplateParam] =
                    t_deductionResult.TemplateArg;

                return Void;
            }));
        }

        ACE_TRY(templateArgs, TransformExpectedVector(t_templateParams,
        [&](Symbol::Type::TemplateParam::Normal* const t_templateParam) -> Expected<Symbol::Type::IBase*>
        {
            const auto foundIt = templateArgMap.find(t_templateParam);
            ACE_TRY_ASSERT(foundIt != end(templateArgMap));
            return foundIt->second;
        }));

        return templateArgs;
    }

    static auto DeduceTemplateArgs(
        Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_templateArgs,
        const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgTypes
    ) -> Expected<std::vector<Symbol::Type::IBase*>>
    {
        const auto templateParams = t_template->CollectParams();
        if (templateParams.size() == t_templateArgs.size())
        {
            return t_templateArgs;
        }

        ACE_TRY_ASSERT(t_optArgTypes.has_value());

        auto* const parametrized = dynamic_cast<Symbol::IParamized*>(
            t_template->GetPlaceholderSymbol()
        );
        ACE_TRY_ASSERT(parametrized);

        const auto paramTypes = parametrized->CollectParamTypes();
        ACE_TRY_ASSERT(!paramTypes.empty());

        return DeduceAlgorithm(
            t_templateArgs,
            templateParams,
            t_optArgTypes.value(),
            paramTypes
        );
    }
    
    auto Scope::ResolveOrInstantiateTemplateInstance(
        const Compilation* const t_compilation,
        Symbol::Template::IBase* const t_template,
        const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgTypes,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArgs,
        const std::vector<Symbol::Type::IBase*>& t_templateArgs
    ) -> Expected<Symbol::IBase*>
    {
        ACE_TRY(deducedTemplateArgs, DeduceTemplateArgs(
            t_template,
            t_templateArgs,
            t_optArgTypes
        ));

        const auto expResolvedInstance = ResolveTemplateInstance(
            t_template,
            t_implTemplateArgs,
            deducedTemplateArgs
        );
        if (expResolvedInstance)
        {
            return expResolvedInstance.Unwrap();
        }

        ACE_TRY(symbol, t_compilation->TemplateInstantiator->InstantiateSymbols(
            t_template,
            t_implTemplateArgs,
            deducedTemplateArgs
        ));

        return symbol;
    }

    auto Scope::CollectTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        auto aliases =
            CollectSymbols<Symbol::Type::Alias::TemplateArg::Normal>();

        std::sort(begin(aliases), end(aliases),
        [](
            const Symbol::Type::Alias::TemplateArg::Normal* const t_lhs,
            const Symbol::Type::Alias::TemplateArg::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Type::IBase*> args{};
        std::transform(
            begin(aliases),
            end  (aliases),
            back_inserter(args),
            [](const Symbol::Type::Alias::TemplateArg::Normal* const t_alias)
            {
                return t_alias->GetAliasedType();
            }
        );

        return args;
    }

    auto Scope::CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*>
    {
        auto aliases =
            CollectSymbols<Symbol::Type::Alias::TemplateArg::Impl>();

        std::sort(begin(aliases), end(aliases),
        [](
            const Symbol::Type::Alias::TemplateArg::Impl* const t_lhs,
            const Symbol::Type::Alias::TemplateArg::Impl* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Type::IBase*> args{};
        std::transform(
            begin(aliases),
            end  (aliases),
            back_inserter(args),
            [](const Symbol::Type::Alias::TemplateArg::Impl* const t_alias)
            {
                return t_alias->GetAliasedType();
            }
        );

        return args;
    }

    auto Scope::DefineTemplateArgAliases(
        const std::vector<std::string>& t_implTemplateParamNames, 
        const std::vector<Symbol::Type::IBase*> t_implTemplateArgs, 
        const std::vector<std::string>& t_templateParamNames, 
        const std::vector<Symbol::Type::IBase*> t_templateArgs
    ) -> Expected<void>
    {
        ACE_TRY_ASSERT(
            t_implTemplateParamNames.size() ==
            t_implTemplateArgs.size()
        );
        ACE_TRY_ASSERT(
            t_templateParamNames.size() ==
            t_templateArgs.size()
        );

        for (size_t i = 0; i < t_implTemplateParamNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<Symbol::Type::Alias::TemplateArg::Impl>(
                shared_from_this(),
                t_implTemplateParamNames.at(i),
                t_implTemplateArgs.at(i),
                i
            );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        for (size_t i = 0; i < t_templateParamNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<Symbol::Type::Alias::TemplateArg::Normal>(
                shared_from_this(),
                t_templateParamNames.at(i),
                t_templateArgs.at(i),
                i
            );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        return Void;
    }

    Scope::Scope(
        const Compilation* const t_compilation
    ) : Scope
        {
            t_compilation,
            std::string{ SpecialIdentifier::Global },
            std::nullopt
        }
    {
    }

    auto Scope::Clear() -> void
    {
        std::for_each(begin(m_Children), end(m_Children),
        [](const std::weak_ptr<Scope>& t_child)
        {
            t_child.lock()->Clear();
        });

        m_SymbolMap.clear();
        m_OptParent = std::nullopt;
    }

    Scope::Scope(
        const Compilation* const t_compilation,
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

#if 0 // This doesnt work for associated functions params, needs rework.
        if (auto typedSymbol = dynamic_cast<const Symbol::IBase*>(t_symbol))
        {
            if (
                (typedSymbol->GetType()->GetAccessModifier() == AccessModifier::Private) && 
                (typedSymbol->GetAccessModifier() != AccessModifier::Private)
                )
                return false;
        }
#endif

        const auto [templateArgs, implTemplateArgs] = [&]() -> std::tuple<std::vector<Symbol::Type::IBase*>, std::vector<Symbol::Type::IBase*>>
        {
            const auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(
                t_symbol
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
            t_symbol->GetName(), 

            templateArgs, 
            implTemplateArgs
        ).has_value();
    }

    auto Scope::GetDefinedSymbol(
        const std::string& t_name,
        const std::vector<Symbol::Type::IBase*>& t_templateArgs,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArgs
    ) -> std::optional<Symbol::IBase*>
    {
        const auto matchingNameSymbolsIt = m_SymbolMap.find(t_name);
        if (matchingNameSymbolsIt == end(m_SymbolMap))
        {
            return std::nullopt;
        }

        const auto& matchingNameSymbols = matchingNameSymbolsIt->second;

        const bool isTemplateInstance = 
            !t_templateArgs.empty() || 
            !t_implTemplateArgs.empty();

        if (isTemplateInstance)
        {
            const auto perfectMatchIt = std::find_if(
                begin(matchingNameSymbols), 
                end  (matchingNameSymbols), 
                [&] (const std::unique_ptr<Symbol::IBase>& t_symbol)
                {
                    auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(
                        t_symbol.get()
                    );
                    ACE_ASSERT(templatableSymbol);

                    const auto templateArgs =
                        templatableSymbol->CollectTemplateArgs();

                    const bool doTemplateArgsMatch = AreTypesSame(
                        templateArgs,
                        t_templateArgs
                    );

                    if (!doTemplateArgsMatch)
                    {
                        return false;
                    }

                    const auto implTemplateArgs =
                        templatableSymbol->CollectImplTemplateArgs();

                    const bool doImplTemplateArgsMatch = AreTypesSame(
                        implTemplateArgs,
                        t_implTemplateArgs
                    );

                    if (!doImplTemplateArgsMatch)
                    {
                        return false;
                    }

                    return true;
                }
            );

            return (perfectMatchIt == end(matchingNameSymbols)) ?
                std::optional<Symbol::IBase*>{} :
                perfectMatchIt->get();
        }
        else
        {
            ACE_ASSERT(matchingNameSymbols.size() == 1);
            return matchingNameSymbols.front().get();
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
        ACE_TRY(templateArgs, ResolveTemplateArgs(
            t_data.ResolvingFromScope,
            t_data.NameSectionsBegin->TemplateArgs
        ));

        return ResolveSymbolInScopes(t_data, templateArgs);
    }

    auto Scope::ResolveSymbolInScopes(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArgs
    ) -> Expected<Symbol::IBase*>
    {
        ACE_ASSERT(
            t_data.NameSectionsBegin->TemplateArgs.empty() || 
            (
                t_templateArgs.size() == 
                t_data.NameSectionsBegin->TemplateArgs.size()
            )
        );

        std::vector<Symbol::IBase*> symbols{};
        std::for_each(begin(t_data.Scopes), end(t_data.Scopes),
        [&](const std::shared_ptr<const Scope>& t_scope)
        {
            const auto matchingSymbols = 
                t_scope->CollectMatchingSymbols(t_data, t_templateArgs);

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
            const auto& selfScopeAssociations =
                selfScopedSymbol->GetSelfScope()->m_Associations;
            scopes.insert(
                end(scopes),
                begin(selfScopeAssociations),
                end  (selfScopeAssociations)
            );

            return ResolveSymbolInScopes(SymbolResolutionData{
                t_data.ResolvingFromScope,
                t_data.NameSectionsBegin + 1,
                t_data.NameSectionsEnd,
                t_data.OptArgTypes,
                t_data.IsCorrectSymbolType,
                scopes,
                t_templateArgs,
                t_data.IsSymbolTemplate
            });
        }
    }

    auto Scope::CollectMatchingSymbols(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArgs
    ) const -> std::vector<Symbol::IBase*>
    {
        const auto remainingNameSectionsSize = std::distance(
            t_data.NameSectionsBegin,
            t_data.NameSectionsEnd
        );
        const bool isLastNameSection = remainingNameSectionsSize == 1;

        const auto matchingTemplateNameSymbolsIt =
            m_SymbolMap.find(t_data.TemplateName);

        const bool isTemplate =
            matchingTemplateNameSymbolsIt != end(m_SymbolMap);

        const auto matchingNameSymbolsIt = isTemplate ? 
            matchingTemplateNameSymbolsIt :
            m_SymbolMap.find(t_data.Name);

        if (matchingNameSymbolsIt == end(m_SymbolMap))
        {
            return {};
        }

        const auto& matchingNameSymbols = matchingNameSymbolsIt->second;

        const bool isInstanceVar = IsInstanceVar(
            t_data,
            matchingNameSymbols
        );

        const bool isTemplateSymbol = 
            isTemplate && isLastNameSection && !isInstanceVar;

        if (isTemplateSymbol)
        {
            return CollectMatchingTemplateSymbols(
                t_data,
                t_templateArgs,
                matchingNameSymbols
            );
        }

        return CollectMatchingNormalSymbols(
            t_data,
            t_templateArgs,
            matchingNameSymbols
        );
    }

    auto Scope::CollectMatchingNormalSymbols(
        const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArgs,
        const std::vector<std::unique_ptr<Symbol::IBase>>& t_matchingNameSymbols
    ) const -> std::vector<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};
        std::transform(
            begin(t_matchingNameSymbols), 
            end  (t_matchingNameSymbols), 
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
        const std::vector<Symbol::Type::IBase*>& t_templateArgs,
        const std::vector<std::unique_ptr<Symbol::IBase>>& t_matchingNameSymbols
    ) const -> std::vector<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};

        ACE_ASSERT(t_matchingNameSymbols.size() == 1);
        auto* const tmplate = dynamic_cast<Symbol::Template::IBase*>(
            t_matchingNameSymbols.front().get()
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
                t_data.OptArgTypes,
                t_data.ImplTemplateArgs,
                t_templateArgs
            );
            if (!expTemplateInstance)
                return {};
            
            symbols.push_back(expTemplateInstance.Unwrap());
        }

        return symbols;
    }

    auto Scope::ResolveTemplateInstance(
        const Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArgs,
        const std::vector<Symbol::Type::IBase*>& t_templateArgs
    ) -> Expected<Symbol::IBase*>
    {
        const auto scope = t_template->GetScope();

        const auto matchingNameSymbolsIt =
            scope->m_SymbolMap.find(t_template->GetASTName());
        ACE_TRY_ASSERT(matchingNameSymbolsIt != end(scope->m_SymbolMap));

        auto& symbols = matchingNameSymbolsIt->second;

        const auto perfectCandidateIt = std::find_if(begin(symbols), end(symbols),
        [&](const std::unique_ptr<Symbol::IBase>& t_symbol)
        {
            auto* const templatableSymbol = dynamic_cast<Symbol::ITemplatable*>(
                t_symbol.get()
            );
            ACE_ASSERT(templatableSymbol);

            const auto collectedTemplateArgs =
                templatableSymbol->CollectTemplateArgs();

            const bool doTemplateArgsMatch = AreTypesSame(
                t_templateArgs,
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
                    t_implTemplateArgs,
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
        const std::string& t_name
    ) const -> Expected<Symbol::Template::IBase*>
    {
        auto matchingNameSymbolsIt = m_SymbolMap.find(t_name);
        ACE_TRY_ASSERT(matchingNameSymbolsIt != end(m_SymbolMap));

        std::vector<Symbol::IBase*> symbols{};
        std::transform(
            begin(matchingNameSymbolsIt->second),
            end  (matchingNameSymbolsIt->second),
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

    auto Scope::CollectInstanceSymbolResolutionImplTemplateArgs(
        Symbol::Type::IBase* const t_selfType
    ) -> std::vector<Symbol::Type::IBase*>
    {
        return t_selfType->CollectTemplateArgs();
    }

    auto Scope::GetStaticSymbolResolutionStartScope(
        const SymbolName& t_name
    ) const -> Expected<std::shared_ptr<const Scope>>
    {
        if (t_name.IsGlobal)
        {
            return std::shared_ptr<const Scope>
            { 
                m_Compilation->GlobalScope.Unwrap()
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
            {
                continue;
            }

            return scope;
        }

        ACE_TRY_UNREACHABLE();
    }

    auto Scope::GetStaticSymbolResolutionImplTemplateArgs(
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

            const auto implTemplateArgs =
                scope->CollectImplTemplateArgs();

            if (!implTemplateArgs.empty())
            {
                return implTemplateArgs;
            }
        }

        return {};
    }
}
