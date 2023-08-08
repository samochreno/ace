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
#include "Diagnostics/BindingDiagnostics.hpp"
#include "SpecialIdent.hpp"
#include "Symbols/All.hpp"
#include "SymbolCreatable.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/ConversionPlaceholderExprBoundNode.hpp"
#include "Compilation.hpp"
#include "TypeConversions.hpp"

namespace Ace
{
    static auto IsSymbolAccessibleFromScope(
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

    auto DiagnoseInaccessibleSymbol(
        const SrcLocation& srcLocation,
        ISymbol* const symbol,
        const std::shared_ptr<const Scope>& beginScope
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        if (!IsSymbolAccessibleFromScope(symbol, beginScope))
        {
            diagnostics.Add(CreateInaccessibleSymbolError(
                srcLocation,
                symbol
            ));
        }

        return Diagnosed<void>{ diagnostics };
    }

    auto IsCorrectSymbolCategory(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory symbolCategory
    ) -> Expected<void>
    {
        DiagnosticBag diagnostics{};

        if (symbol->GetCategory() != symbolCategory)
        {
            diagnostics.Add(CreateIncorrectSymbolCategoryError(
                srcLocation,
                symbol,
                symbolCategory
            ));
        }

        return Void{ diagnostics };
    }

    auto CastToTemplatableSymbol(
        const ISymbol* const symbol
    ) -> const ITemplatableSymbol*
    {
        return dynamic_cast<const ITemplatableSymbol*>(symbol);
    }

    auto CollectTemplateArgs(
        const ITemplatableSymbol* const templatableSymbol
    ) -> std::vector<ITypeSymbol*>
    {
        return templatableSymbol->CollectTemplateArgs();
    }

    auto CollectImplTemplateArgs(
        const ITemplatableSymbol* const templatableSymbol
    ) -> std::vector<ITypeSymbol*>
    {
        return templatableSymbol->CollectImplTemplateArgs();
    }

    GlobalScope::GlobalScope()
    {
    }

    GlobalScope::GlobalScope(Compilation* const compilation)
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

    auto Scope::GetCompilation() const -> Compilation*
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
    ) -> Diagnosed<ISymbol*>
    {
        DiagnosticBag diagnostics{};

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
                diagnostics.Collect(partiallyCreatable->ContinueCreatingSymbol(
                    optDefinedSymbol.value()
                ));

                return Diagnosed{ optDefinedSymbol.value(), diagnostics };
            }
        }

        auto* const symbol = diagnostics.Collect(DefineSymbol(
            diagnostics.Collect(creatable->CreateSymbol())
        ));

        return Diagnosed{ symbol, diagnostics };
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
        NormalTemplateParamTypeSymbol* Param{};
        ITypeSymbol* Arg{};
    };

    static auto DeduceTemplateArg(
        ITypeSymbol* argType,
        ITypeSymbol* paramType
    ) -> std::vector<TemplateArgDeductionResult>
    {
        argType = argType->GetWithoutRef();
        paramType = paramType->GetWithoutRef();

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
        if (!optArgTypeTemplate.has_value())
        {
            return {};
        }

        const auto optParamTypeTemplate = paramType->GetTemplate();
        if (!optParamTypeTemplate.has_value())
        {
            return {};
        }

        const auto argTypeTemplateParams = argType->CollectTemplateArgs();
        const auto paramTypeTemplateParams = paramType->CollectTemplateArgs();

        std::vector<TemplateArgDeductionResult> finalDeductionResults{};
        const auto paramsSize = argTypeTemplateParams.size();
        for (size_t i = 0; i < paramsSize; i++)
        {
            auto* const argTypeTemplateParam = argTypeTemplateParams.at(i);
            auto* const paramTypeTemplateParam = paramTypeTemplateParams.at(i);

            const auto deductionResults = DeduceTemplateArg(
                argTypeTemplateParam,
                paramTypeTemplateParam
            );
            
            finalDeductionResults.insert(
                end(finalDeductionResults),
                begin(deductionResults),
                end  (deductionResults)
            );
        }

        return finalDeductionResults;
    }

    static auto CollectTemplateArgsFromMap(
        const SrcLocation& srcLocation, 
        const std::map<NormalTemplateParamTypeSymbol*, ITypeSymbol*>& templateParamToArgMap,
        const std::vector<NormalTemplateParamTypeSymbol*>& templateParams
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        DiagnosticBag diagnostics{};

        std::vector<ITypeSymbol*> templateArgs{};
        std::for_each(begin(templateParams), end(templateParams),
        [&](NormalTemplateParamTypeSymbol* const templateParam)
        {
            const auto matchingTemplateArgIt = templateParamToArgMap.find(
                templateParam
            );
            const bool hasMatchingTemplateArg = 
                matchingTemplateArgIt != end(templateParamToArgMap);

            if (!hasMatchingTemplateArg)
            {
                diagnostics.Add(CreateUnableToDeduceTemplateArgError(
                    srcLocation,
                    templateParam
                ));
            }

            auto* compilation = srcLocation.Buffer->GetCompilation();

            auto* const templateArg = hasMatchingTemplateArg ?
                matchingTemplateArgIt->second :
                compilation->GetErrorSymbols().GetType();

            templateArgs.push_back(templateArg);
        });

        if (diagnostics.HasErrors())
        {
            return diagnostics;
        }

        return Expected{ templateArgs, diagnostics };
    }

    static auto VerifyTemplateArgDeductionResult(
        const SrcLocation& srcLocation,
        const TemplateArgDeductionResult& deductionResult,
        const std::map<NormalTemplateParamTypeSymbol*, ITypeSymbol*>& paramToArgMap
    ) -> Expected<void>
    {
        DiagnosticBag diagnostics{};

        const auto deducedArgIt = paramToArgMap.find(deductionResult.Param);
        const bool isAlreadyDeduced = deducedArgIt != end(paramToArgMap);
        if (isAlreadyDeduced)
        {
            auto* const alreadyDeducedArg = deducedArgIt->second;

            if (alreadyDeducedArg != deductionResult.Arg)
            {
                return diagnostics.Add(CreateTemplateArgDeductionConflict(
                    srcLocation,
                    deductionResult.Param,
                    alreadyDeducedArg,
                    deductionResult.Arg
                ));
            }
        }

        return Void{ diagnostics };
    }

    static auto CreateKnownTemplateParamToArgMap(
        const std::vector<NormalTemplateParamTypeSymbol*>& templateParams,
        const std::vector<ITypeSymbol*>& knownTemplateArgs
    ) -> std::map<NormalTemplateParamTypeSymbol*, ITypeSymbol*>
    {
        std::map<NormalTemplateParamTypeSymbol*, ITypeSymbol*> map{};

        for (size_t i = 0; i < knownTemplateArgs.size(); i++)
        {
            map[templateParams.at(i)] = knownTemplateArgs.at(i);
        }

        return map;
    }

    static auto TemplateArgDeductionAlgorithm(
        const SrcLocation& srcLocation,
        std::vector<ITypeSymbol*> knownTemplateArgs,
        const std::vector<NormalTemplateParamTypeSymbol*>& templateParams,
        const std::vector<ITypeSymbol*>& argTypes,
        const std::vector<ITypeSymbol*>& paramTypes
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        DiagnosticBag diagnostics{};

        if (knownTemplateArgs.size() > templateParams.size())
        {
            knownTemplateArgs.erase(
                begin(knownTemplateArgs) + templateParams.size(),
                end  (knownTemplateArgs)
            );
            diagnostics.Add(CreateTooManyTemplateArgsError(srcLocation));
        }

        auto templateParamToArgMap = CreateKnownTemplateParamToArgMap(
            templateParams,
            knownTemplateArgs
        );

        std::vector<TemplateArgDeductionResult> deductionResults{};
        for (size_t i = 0; i < argTypes.size(); i++)
        {
            const auto currentDeductionResults = DeduceTemplateArg(
                argTypes.at(i),
                paramTypes.at(i)
            );
            deductionResults.insert(
                end(deductionResults),
                begin(currentDeductionResults),
                end  (currentDeductionResults)
            );
        }

        std::for_each(begin(deductionResults), end(deductionResults),
        [&](const TemplateArgDeductionResult& deductionResult)
        {
            const auto didVerifyDeductionResult = diagnostics.Collect(VerifyTemplateArgDeductionResult(
                srcLocation,
                deductionResult,
                templateParamToArgMap
            ));
            if (!didVerifyDeductionResult)
            {
                return;
            }

            templateParamToArgMap[deductionResult.Param] = deductionResult.Arg;
        });

        const auto optTemplateArgs = diagnostics.Collect(CollectTemplateArgsFromMap(
            srcLocation,
            templateParamToArgMap,
            templateParams
        ));
        if (!optTemplateArgs.has_value())
        {
            return diagnostics;
        }

        return Expected{ optTemplateArgs.value(), diagnostics };
    }

    static auto DeduceTemplateArgs(
        const SrcLocation& srcLocation,
        ITemplateSymbol* const t3mplate,
        const std::vector<ITypeSymbol*>& knownTemplateArgs,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        DiagnosticBag diagnostics{};

        const auto templateParams = t3mplate->CollectParams();
        if (templateParams.size() == knownTemplateArgs.size())
        {
            return knownTemplateArgs;
        }

        if (!optArgTypes.has_value())
        {
            return diagnostics.Add(CreateUnableToDeduceTemplateArgsError(
                srcLocation
            ));
        }

        auto* const parametrized = dynamic_cast<IParamizedSymbol*>(
            t3mplate->GetPlaceholderSymbol()
        );
        if (!parametrized)
        {
            return diagnostics.Add(CreateUnableToDeduceTemplateArgsError(
                srcLocation
            ));
        }

        const auto paramTypes = parametrized->CollectParamTypes();
        if (paramTypes.empty())
        {
            return diagnostics.Add(CreateUnableToDeduceTemplateArgsError(
                srcLocation
            ));
        }

        const auto optTemplateArg = diagnostics.Collect(TemplateArgDeductionAlgorithm(
            srcLocation,
            knownTemplateArgs,
            templateParams,
            optArgTypes.value(),
            paramTypes
        ));
        if (!optTemplateArg.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            optTemplateArg.value(),
            diagnostics,
        };
    }
    
    auto Scope::ResolveOrInstantiateTemplateInstance(
        const SrcLocation& srcLocation,
        ITemplateSymbol* const t3mplate,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
        const std::vector<ITypeSymbol*>& implTemplateArgs,
        const std::vector<ITypeSymbol*>& templateArgs
    ) -> Expected<ISymbol*>
    {
        DiagnosticBag diagnostics{};

        const auto optDeducedTemplateArgs = diagnostics.Collect(DeduceTemplateArgs(
            srcLocation,
            t3mplate,
            templateArgs,
            optArgTypes
        ));
        if (!optDeducedTemplateArgs.has_value())
        {
            return diagnostics;
        }

        const auto optResolvedInstance = ResolveTemplateInstance(
            srcLocation,
            t3mplate,
            implTemplateArgs,
            optDeducedTemplateArgs.value()
        );
        if (optResolvedInstance)
        {
            return Expected{ optResolvedInstance.value(), diagnostics };
        }

        auto* const compilation = t3mplate->GetCompilation();

        auto* const symbol = diagnostics.Collect(compilation->GetTemplateInstantiator().InstantiateSymbols(
            t3mplate,
            implTemplateArgs,
            optDeducedTemplateArgs.value()
        ));

        return Expected{ symbol, diagnostics };
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
        auto aliases = CollectSymbols<ImplTemplateArgAliasTypeSymbol>();

        std::sort(begin(aliases), end(aliases),
        [](
            const ImplTemplateArgAliasTypeSymbol* const lhs,
            const ImplTemplateArgAliasTypeSymbol* const rhs
            )
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        std::vector<ITypeSymbol*> args{};
        std::transform(begin(aliases), end(aliases), back_inserter(args),
        [](const ImplTemplateArgAliasTypeSymbol* const alias)
        {
            return alias->GetAliasedType();
        });

        return args;
    }

    auto Scope::DefineTemplateArgAliases(
        const std::vector<Ident>& implTemplateParamNames, 
        const std::vector<ITypeSymbol*> implTemplateArgs, 
        const std::vector<Ident>& templateParamNames, 
        const std::vector<ITypeSymbol*> templateArgs
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        ACE_ASSERT(implTemplateParamNames.size() == implTemplateArgs.size());
        ACE_ASSERT(templateParamNames.size() == templateArgs.size());

        for (size_t i = 0; i < implTemplateParamNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<ImplTemplateArgAliasTypeSymbol>(
                shared_from_this(),
                implTemplateParamNames.at(i),
                implTemplateArgs.at(i),
                i
            );

            (void)diagnostics.Collect(DefineSymbol(std::move(aliasSymbol)));
        }

        for (size_t i = 0; i < templateParamNames.size(); i++)
        {
            auto aliasSymbol = std::make_unique<NormalTemplateArgAliasTypeSymbol>(
                shared_from_this(),
                templateParamNames.at(i),
                templateArgs.at(i),
                i
            );

            (void)diagnostics.Collect(DefineSymbol(std::move(aliasSymbol)));
        }

        return Diagnosed<void>{ diagnostics };
    }

    Scope::Scope(
        Compilation* const compilation
    ) : Scope
        {
            compilation,
            std::string{ SpecialIdent::Global },
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
        Compilation* const compilation,
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
            SpecialIdent::CreateAnonymous();
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
            !templateArgs.empty() || !implTemplateArgs.empty();

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

    static auto ResolveLastNameSectionSymbol(
        const SymbolResolutionData& data,
        const std::vector<ISymbol*>& matchingSymbols
    ) -> Expected<ISymbol*>
    {
        DiagnosticBag diagnostics{};

        if (matchingSymbols.empty())
        {
            return diagnostics.Add(CreateUndefinedSymbolRefError(
                data.SrcLocation
            ));
        }

        std::vector<ISymbol*> symbols{};
        std::copy_if(
            begin(matchingSymbols), 
            end  (matchingSymbols), 
            back_inserter(symbols), 
            [&](ISymbol* const matchingSymbol)
            {
                const bool isCorrectSymbolType = data.IsCorrectSymbolType(
                    data.SrcLocation,
                    matchingSymbol
                );
                if (!isCorrectSymbolType)
                {
                    return false;
                }

                return true;
            }
        );

        if (symbols.empty())
        {
            const bool isCorrectSymbolType = diagnostics.Collect(data.IsCorrectSymbolType(
                data.SrcLocation,
                matchingSymbols.front()
            ));
            if (!isCorrectSymbolType)
            {
                return diagnostics;
            }
        }

        if (symbols.size() > 1)
        {
            diagnostics.Add(CreateAmbiguousSymbolRefError(
                data.SrcLocation,
                symbols
            ));
        }

        auto* const symbol = symbols.front();
        diagnostics.Collect(DiagnoseInaccessibleSymbol(
            data.SrcLocation,
            symbol,
            data.BeginScope
        ));

        return Expected{ symbols.front(), diagnostics };
    }

    static auto HasResolvedTemplateArgs(
        const SymbolResolutionData& data
    ) -> bool
    {
        return
            !data.TemplateArgs.empty() ||
            data.NameSectionsBegin->TemplateArgs.empty();
    }

    static auto ResolveTemplateArgs(
        const SymbolResolutionData& data
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        DiagnosticBag diagnostics{};

        const auto& scope = data.BeginScope;
        const auto& argNames = data.NameSectionsBegin->TemplateArgs;

        std::vector<ITypeSymbol*> args{};
        std::transform(begin(argNames), end(argNames), back_inserter(args),
        [&](const SymbolName& argName)
        {
            const auto optArg = diagnostics.Collect(scope->ResolveStaticSymbol<ITypeSymbol>(
                argName
            ));
            return optArg.value_or(
                scope->GetCompilation()->GetErrorSymbols().GetType()
            );
        });

        if (diagnostics.HasErrors())
        {
            return diagnostics;
        }

        return Expected{ args, diagnostics };
    }

    static auto HasSrcLocation(
        const SymbolResolutionData& data
    ) -> bool
    {
        return data.SrcLocation.Buffer != nullptr;
    }


    auto Scope::ResolveSymbolInScopes(
        SymbolResolutionData data
    ) -> Expected<ISymbol*>
    {
        DiagnosticBag diagnostics{};

        if (!HasResolvedTemplateArgs(data))
        {
            const auto optTemplateArgs = diagnostics.Collect(
                ResolveTemplateArgs(data)
            );
            if (!optTemplateArgs.has_value())
            {
                return diagnostics;
            }

            data.TemplateArgs = optTemplateArgs.value();
        }

        const auto optMatchingSymbols = diagnostics.Collect(
            CollectMatchingSymbolsInScopes(data)
        );
        if (!optMatchingSymbols.has_value())
        {
            return diagnostics;
        }

        const auto optSymbol = diagnostics.Collect(data.IsLastNameSection ? 
            ResolveLastNameSectionSymbol(data, optMatchingSymbols.value()) :
            ResolveNameSectionSymbol    (data, optMatchingSymbols.value())
        );
        if (!optSymbol.has_value())
        {
            return diagnostics;
        }

        if (diagnostics.HasErrors())
        {
            return diagnostics;
        }

        return Expected{ optSymbol.value(), diagnostics };
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
        return dynamic_cast<InstanceVarSymbol*>(symbol) != nullptr;
    }

    auto Scope::CollectMatchingSymbolsInScopes(
        const SymbolResolutionData& data
    ) -> Expected<std::vector<ISymbol*>>
    {
        DiagnosticBag diagnostics{};

        std::vector<ISymbol*> matchingSymbols{};
        std::for_each(begin(data.Scopes), end(data.Scopes),
        [&](const std::shared_ptr<const Scope>& scope)
        {
            const auto optMatchingSymbols = diagnostics.Collect(
                scope->CollectMatchingSymbols(data)
            );
            if (!optMatchingSymbols.has_value())
            {
                return;
            }

            matchingSymbols.insert(
                end(matchingSymbols),
                begin(optMatchingSymbols.value()),
                end  (optMatchingSymbols.value())
            );
        });

        if (matchingSymbols.empty() && diagnostics.HasErrors())
        {
            return diagnostics;
        }

        return Expected{ matchingSymbols, diagnostics };
    }

    auto Scope::CollectMatchingSymbols(
        const SymbolResolutionData& data
    ) const -> Expected<std::vector<ISymbol*>>
    {
        DiagnosticBag diagnostics{};

        const auto matchingTemplateNameSymbolsIt =
            m_SymbolMap.find(data.TemplateName);

        const bool isTemplate =
            matchingTemplateNameSymbolsIt != end(m_SymbolMap);

        const auto matchingNameSymbolsIt = isTemplate ? 
            matchingTemplateNameSymbolsIt :
            m_SymbolMap.find(data.Name);

        if (matchingNameSymbolsIt == end(m_SymbolMap))
        {
            return Expected{ std::vector<ISymbol*>{}, diagnostics };
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
                matchingNameSymbols
            );
        }
        else
        {
            return CollectMatchingNormalSymbols(
                data,
                matchingNameSymbols
            );
        }
    }

    auto Scope::CollectMatchingNormalSymbols(
        const SymbolResolutionData& data,
        const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
    ) const -> Expected<std::vector<ISymbol*>>
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

        return Expected
        {
            symbols,
            DiagnosticBag{},
        };
    }

    auto Scope::CollectMatchingTemplateSymbols(
        const SymbolResolutionData& data,
        const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
    ) const -> Expected<std::vector<ISymbol*>>
    {
        DiagnosticBag diagnostics{};

        auto* const t3mplate = dynamic_cast<ITemplateSymbol*>(
            matchingNameSymbols.front().get()
        );
        ACE_ASSERT(t3mplate);

        if (data.IsTemplate)
        {
            return Expected
            {
                std::vector<ISymbol*>{ t3mplate },
                diagnostics,
            };
        }

        const auto optTemplateInstance = diagnostics.Collect(ResolveOrInstantiateTemplateInstance(
            data.SrcLocation,
            t3mplate,
            data.OptArgTypes,
            data.ImplTemplateArgs,
            data.TemplateArgs
        ));
        if (!optTemplateInstance.has_value())
        {
            return diagnostics;
        }

        return Expected
        {
            std::vector<ISymbol*>{ optTemplateInstance.value() },
            diagnostics,
        };
    }

    static auto DoTemplateInstanceArgsMatch(
        ISymbol* const templateInstanceSymbol,
        const std::vector<ITypeSymbol*>& templateArgs,
        const std::vector<ITypeSymbol*>& implTemplateArgs
    ) -> bool
    {
        auto* const templatableSymbol =
            dynamic_cast<ITemplatableSymbol*>(templateInstanceSymbol);
        ACE_ASSERT(templatableSymbol);

        const bool doTemplateArgsMatch = AreTypesSame(
            templateArgs,
            templatableSymbol->CollectTemplateArgs()
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
    }

    auto Scope::ResolveTemplateInstance(
        const SrcLocation& srcLocation,
        const ITemplateSymbol* const t3mplate,
        const std::vector<ITypeSymbol*>& implTemplateArgs,
        const std::vector<ITypeSymbol*>& templateArgs
    ) -> std::optional<ISymbol*>
    {
        const auto& symbolMap = t3mplate->GetScope()->m_SymbolMap;

        const auto matchingNameSymbolsIt = symbolMap.find(
            t3mplate->GetASTName().String
        );
        if (matchingNameSymbolsIt == end(symbolMap))
        {
            return std::nullopt;
        }

        auto& symbols = matchingNameSymbolsIt->second;

        const auto perfectMatchIt = std::find_if(
            begin(symbols),
            end  (symbols),
            [&](const std::unique_ptr<ISymbol>& symbol)
            {
                return DoTemplateInstanceArgsMatch(
                    symbol.get(),
                    templateArgs,
                    implTemplateArgs
                );
            }
        );
        if (perfectMatchIt == end(symbols))
        {
            return std::nullopt;
        }

        return perfectMatchIt->get();
    }

    auto Scope::GetInstanceSymbolResolutionScopes(
        ITypeSymbol* selfType
    ) -> std::vector<std::shared_ptr<const Scope>>
    {
        selfType = selfType->GetUnaliased();

        if (selfType->IsRef())
        {
            return GetInstanceSymbolResolutionScopes(
                selfType->GetWithoutRef()
            );
        }

        if (selfType->IsStrongPtr())
        {
            return GetInstanceSymbolResolutionScopes(
                selfType->GetWithoutStrongPtr()
            );
        }

        const auto typeSelfScope = selfType->GetSelfScope();
        
        std::vector<std::shared_ptr<const Scope>> scopes{};
        scopes.push_back(typeSelfScope);

        const auto optTemplate = selfType->GetTemplate();
        const auto& associationsScope = optTemplate.has_value() ?
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

    auto Scope::FindStaticSymbolResolutionBeginScope(
        const SrcLocation& srcLocation,
        const SymbolName& name
    ) const -> Expected<std::shared_ptr<const Scope>>
    {
        DiagnosticBag diagnostics{};

        if (name.IsGlobal)
        {
            return Expected
            {
                m_Compilation->GetGlobalScope(),
                diagnostics,
            };
        }

        const auto& section = name.Sections.front();

        const auto& nameString = section.Name.String;
        const auto templateNameString = SpecialIdent::CreateTemplate(
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

            return Expected{ scope, diagnostics };
        }

        return diagnostics.Add(CreateUndefinedSymbolRefError(srcLocation));
    }

    auto Scope::GetStaticSymbolResolutionImplTemplateArgs(
        const std::shared_ptr<const Scope>& beginScope
    ) -> std::vector<ITypeSymbol*>
    {
        for (
            std::optional<std::shared_ptr<const Scope>> optScope = beginScope;
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

    auto Scope::ResolveNameSectionSymbol(
        const SymbolResolutionData& data,
        const std::vector<ISymbol*>& matchingSymbols
    ) -> Expected<ISymbol*>
    {
        DiagnosticBag diagnostics{};

        if (matchingSymbols.empty())
        {
            return diagnostics.Add(CreateUndefinedSymbolRefError(
                data.SrcLocation
            ));
        }

        if (matchingSymbols.size() > 1)
        {
            diagnostics.Add(CreateAmbiguousSymbolRefError(
                data.SrcLocation,
                matchingSymbols
            ));
        }

        auto* const selfScopedSymbol = dynamic_cast<ISelfScopedSymbol*>(
            matchingSymbols.front()
        );
        if (!selfScopedSymbol)
        {
            return diagnostics.Add(CreateScopeAccessOfNonSelfScopedSymbolError(
                data.SrcLocation,
                matchingSymbols.front()
            ));
        }

        const auto optSymbol = diagnostics.Collect(ResolveSymbolInScopes(SymbolResolutionData{
            (data.NameSectionsBegin + 1)->CreateSrcLocation(),
            data.BeginScope,
            data.NameSectionsBegin + 1,
            data.NameSectionsEnd,
            data.OptArgTypes,
            data.IsCorrectSymbolType,
            selfScopedSymbol->GetSelfScope()->CollectSelfAndAssociations(),
            data.TemplateArgs,
            data.IsTemplate,
        }));   
        if (!optSymbol.has_value())
        {
            return diagnostics;
        }

        return Expected{ optSymbol.value(), diagnostics };
    }

    auto Scope::CollectSelfAndAssociations() const -> std::vector<std::shared_ptr<const Scope>>
    {
        std::vector<std::shared_ptr<const Scope>> scopes{};
        scopes.push_back(shared_from_this());
        scopes.insert(
            end(scopes),
            begin(m_Associations),
            end  (m_Associations)
        );

        return scopes;
    }
}
