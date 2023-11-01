#include "Scope.hpp"

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <map>

#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "AnonymousIdent.hpp"
#include "Symbols/All.hpp"
#include "Decl.hpp"
#include "Compilation.hpp"
#include "TypeConversions.hpp"
#include "Keyword.hpp"
#include "PlaceholderOverlapping.hpp"
#include "GenericInstantiator.hpp"

namespace Ace
{
    static auto IsSymbolAccessibleFromScope(
        ISymbol* const symbol,
        const std::shared_ptr<const Scope>& scope
    ) -> bool
    {
        switch (symbol->GetAccessModifier())
        {
            case AccessModifier::Priv:
            {
                const auto optSymbolMod = symbol->GetScope()->FindMod();
                if (!optSymbolMod.has_value())
                {
                    return true; 
                }

                auto* const symbolMod = optSymbolMod.value();

                const auto optScopeMod = scope->FindMod();
                if (!optScopeMod.has_value())
                {
                    return false;
                }

                auto* const scopeMod = optScopeMod.value();
                if (symbolMod == scopeMod)
                {
                    return true;
                }

                const auto symbolModBodyScope = symbolMod->GetBodyScope();
                const bool isSymbolChildOfScope =
                    symbolModBodyScope->HasChild(scopeMod->GetBodyScope());

                if (isSymbolChildOfScope)
                {
                    return true;
                }

                return false;
            }

            case AccessModifier::Pub:
            {
                return true;
            }
        }
    }

    auto DiagnoseInaccessibleSymbol(
        const SrcLocation& srcLocation,
        ISymbol* const symbol,
        const std::shared_ptr<const Scope>& beginScope
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!IsSymbolAccessibleFromScope(symbol, beginScope))
        {
            diagnostics.Add(CreateInaccessibleSymbolError(srcLocation, symbol));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto GetUnaliasedSymbol(ISymbol* const symbol) -> ISymbol*
    {
        return symbol->GetUnaliased();
    }

    auto IsCorrectSymbolCategory(
        const SrcLocation& srcLocation,
        const ISymbol* const symbol,
        const SymbolCategory symbolCategory
    ) -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (symbol->GetCategory() != symbolCategory)
        {
            diagnostics.Add(CreateIncorrectSymbolCategoryError(
                srcLocation,
                symbol,
                symbolCategory
            ));
            return std::move(diagnostics);
        }

        return Void{ std::move(diagnostics) };
    }

    auto CastToGeneric(
        const ISymbol* const symbol
    ) -> const IGenericSymbol*
    {
        return dynamic_cast<const IGenericSymbol*>(symbol);
    }

    auto CastToGeneric(
        const ITypeSymbol* const symbol
    ) -> const IGenericSymbol*
    {
        return dynamic_cast<const IGenericSymbol*>(symbol);
    }

    auto GetTypeArgs(
        const IGenericSymbol* const generic
    ) -> const std::vector<ITypeSymbol*>&
    {
        return generic->GetTypeArgs();
    }

    auto GetDerefed(ITypeSymbol* const type) -> ITypeSymbol*
    {
        return type->GetDerefed();
    }

    auto GetPrototypeSelfType(
        const ISymbol* const symbol
    ) -> std::optional<ITypeSymbol*>
    {
        auto* const prototype =
            dynamic_cast<const PrototypeSymbol*>(symbol->GetUnaliased());
        if (!prototype)
        {
            return std::nullopt;
        }

        return prototype->GetSelfType();
    }

    auto SymbolResolutionContext::IsLastNameSection() const -> bool
    {
        return std::distance(NameSection, NameSectionsEnd) == 1;
    }

    auto SymbolResolutionContext::GetName() const -> const std::string&
    {
        return NameSection->Name.String;
    }

    GlobalScope::GlobalScope()
    {
    }

    GlobalScope::GlobalScope(
        Compilation* const compilation
    ) : m_Scope{ new Scope(compilation) }
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
        return std::find_if(begin(children), end(children),
        [&](const std::weak_ptr<Scope>& child)
        {
            return child.expired();
        });
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

    auto Scope::GetName() const -> const std::optional<std::string>&
    {
        return m_OptName;
    }

    auto Scope::GetAnonymousName() const -> const std::optional<std::string>&
    {
        return m_OptAnonymousName;
    }

    auto Scope::GetGenericInstantiator() -> GenericInstantiator&
    {
        return m_GenericInstantiator;
    }

    auto Scope::FindMod() const -> std::optional<ModSymbol*>
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

            if (!child->GetName().has_value())
            {
                continue;
            }

            const auto matchingNameSymbolsIt =
                parent->m_SymbolMap.find(child->GetName().value());

            if (matchingNameSymbolsIt == end(parent->m_SymbolMap))
            {
                continue;
            }

            const auto& matchingNameSymbols = matchingNameSymbolsIt->second;

            if (matchingNameSymbols.size() != 1)
            {
                continue;
            }

            auto* const modSymbol =
                dynamic_cast<ModSymbol*>(matchingNameSymbols.front().get());
            if (!modSymbol)
            {
                continue;
            }

            return modSymbol;
        }

        return std::nullopt;
    }

    auto Scope::FindPackageMod() const -> ModSymbol*
    {
        auto scope = shared_from_this();
        while (scope->GetParent().value()->GetParent().has_value())
        {
            scope = scope->GetParent().value();
        }

        return scope->FindMod().value();
    }

    auto Scope::CreateChild() -> std::shared_ptr<Scope>
    {
        return AddChild(std::nullopt);
    }

    auto Scope::GetOrCreateChild(
        const std::string& name
    ) -> std::shared_ptr<Scope>
    {
        const auto matchingNameChildIt = std::find_if(
            begin(m_Children),
            end  (m_Children),
            [&](const std::weak_ptr<Scope>& child)
            {
                return child.lock()->m_OptName == name;
            }
        );
        if (matchingNameChildIt != end(m_Children))
        {
            return matchingNameChildIt->lock();
        }

        return AddChild(name);
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

    auto Scope::DeclareSymbol(const IDecl* const decl) -> Diagnosed<ISymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto scope = decl->GetSymbolScope();

        if (auto* const partialDecl = dynamic_cast<const IPartialDecl*>(decl))
        {
            const auto optDeclaredSymbol = scope->GetDeclaredSymbol(
                partialDecl->GetName().String,
                {},
                std::nullopt
            );

            if (optDeclaredSymbol.has_value())
            {
                diagnostics.Collect(partialDecl->ContinueCreatingSymbol(
                    optDeclaredSymbol.value()
                ));
                return Diagnosed
                {
                    optDeclaredSymbol.value(),
                    std::move(diagnostics),
                };
            }
        }

        auto* const symbol = diagnostics.Collect(
            DeclareSymbol(diagnostics.Collect(decl->CreateSymbol()))
        );

        return Diagnosed{ symbol, std::move(diagnostics) };
    }

    auto Scope::CollectChildren() const -> std::vector<std::shared_ptr<Scope>>
    {
        std::vector<std::shared_ptr<Scope>> children;
        std::transform(
            begin(m_Children),
            end  (m_Children),
            back_inserter(children),
            [](const std::weak_ptr<Scope>& child) { return child.lock(); }
        );

        return children;
    }

    auto Scope::HasSymbolWithName(const std::string& name) const -> bool
    {
        return m_SymbolMap.find(name) != end(m_SymbolMap);
    }

    auto Scope::RemoveSymbol(ISymbol* const symbol) -> void
    {
        const auto scope = symbol->GetScope();
        auto& symbols = scope->m_SymbolMap.at(symbol->GetName().String);

        const auto matchingSymbolIt = std::find_if(begin(symbols), end(symbols),
        [&](const std::unique_ptr<ISymbol>& ownedSymbol)
        {
            return ownedSymbol.get() == symbol;
        });
        ACE_ASSERT(matchingSymbolIt != end(symbols));

        symbols.erase(matchingSymbolIt);
    }

    auto Scope::CreateArgTypes(
        const std::vector<ITypeSymbol*>& argTypes
    ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>
    {
        return { argTypes };
    }

    auto Scope::CollectAllSymbols() const -> std::vector<ISymbol*>
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

    auto Scope::CollectAllSymbolsRecursive() const -> std::vector<ISymbol*>
    {
        auto symbols = CollectAllSymbols();

        std::for_each(begin(m_Children), end(m_Children),
        [&](const std::weak_ptr<Scope>& child)
        {
            const auto childSymbols =
                child.lock()->CollectAllSymbolsRecursive();

            symbols.insert(
                end(symbols),
                begin(childSymbols),
                end  (childSymbols)
            );
        });

        return symbols;
    }

    struct TypeArgDeductionResult
    {
        TypeParamTypeSymbol* Param{};
        ITypeSymbol* Arg{};
    };

    static auto DeduceTypeArg(
        ITypeSymbol* argType,
        ITypeSymbol* paramType
    ) -> std::vector<TypeArgDeductionResult>
    {
          argType =   argType->GetWithoutRef()->GetUnaliasedType();
        paramType = paramType->GetWithoutRef()->GetUnaliasedType();

        auto* const typeParam = dynamic_cast<TypeParamTypeSymbol*>(paramType);
        if (typeParam)
        {
            return std::vector{ TypeArgDeductionResult{ typeParam, argType } };
        }

        const auto& argTypeArgs   =   argType->GetTypeArgs();
        const auto& paramTypeArgs = paramType->GetTypeArgs();

        if (argTypeArgs.size() != paramTypeArgs.size())
        {
            return {};
        }

        std::vector<TypeArgDeductionResult> finalDeductionResults{};
        for (size_t i = 0; i < argTypeArgs.size(); i++)
        {
            const auto deductionResults =
                DeduceTypeArg(argTypeArgs.at(i), paramTypeArgs.at(i));
            
            finalDeductionResults.insert(
                end(finalDeductionResults),
                begin(deductionResults),
                end  (deductionResults)
            );
        }

        return finalDeductionResults;
    }

    static auto CollectTypeArgsFromMap(
        const SrcLocation& srcLocation, 
        const std::map<TypeParamTypeSymbol*, ITypeSymbol*>& paramToArgMap,
        const std::vector<TypeParamTypeSymbol*>& params
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<ITypeSymbol*> args{};
        std::for_each(begin(params), end(params),
        [&](TypeParamTypeSymbol* const param)
        {
            const auto matchingArgIt = paramToArgMap.find(param);
            const bool hasMatchingArg = matchingArgIt != end(paramToArgMap);

            if (!hasMatchingArg)
            {
                diagnostics.Add(CreateUnableToDeduceTypeArgError(
                    srcLocation,
                    param
                ));
            }

            auto* const compilation = srcLocation.Buffer->GetCompilation();

            auto* const arg = hasMatchingArg ?
                matchingArgIt->second :
                compilation->GetErrorSymbols().GetType();

            args.push_back(arg);
        });

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Expected{ args, std::move(diagnostics) };
    }

    static auto VerifyTypeArgDeductionResult(
        const SrcLocation& srcLocation,
        const TypeArgDeductionResult& deductionResult,
        const std::map<TypeParamTypeSymbol*, ITypeSymbol*>& paramToArgMap
    ) -> Expected<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto deducedArgIt = paramToArgMap.find(deductionResult.Param);
        const bool isAlreadyDeduced = deducedArgIt != end(paramToArgMap);
        if (isAlreadyDeduced)
        {
            auto* const alreadyDeducedArg = deducedArgIt->second;

            if (alreadyDeducedArg != deductionResult.Arg)
            {
                diagnostics.Add(CreateTypeArgDeductionConflict(
                    srcLocation,
                    deductionResult.Param,
                    alreadyDeducedArg,
                    deductionResult.Arg
                ));
                return std::move(diagnostics);
            }
        }

        return Void{ std::move(diagnostics) };
    }

    static auto CreateKnownTypeParamToArgMap(
        const std::vector<TypeParamTypeSymbol*>& params,
        const std::vector<ITypeSymbol*>& knownArgs
    ) -> std::map<TypeParamTypeSymbol*, ITypeSymbol*>
    {
        std::map<TypeParamTypeSymbol*, ITypeSymbol*> map{};

        for (size_t i = 0; i < knownArgs.size(); i++)
        {
            map[params.at(i)] = knownArgs.at(i);
        }

        return map;
    }

    static auto TypeArgDeductionAlgorithm(
        const SrcLocation& srcLocation,
        std::vector<ITypeSymbol*> knownArgs,
        const std::vector<TypeParamTypeSymbol*>& params,
        const std::vector<ITypeSymbol*>& argTypes,
        const std::vector<ITypeSymbol*>& paramTypes
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (knownArgs.size() > params.size())
        {
            knownArgs.erase(begin(knownArgs) + params.size(), end(knownArgs));
            diagnostics.Add(CreateTooManyTypeArgsError(srcLocation));
        }

        auto paramToArgMap = CreateKnownTypeParamToArgMap(params, knownArgs);

        std::vector<TypeArgDeductionResult> deductionResults{};
        for (size_t i = 0; i < argTypes.size(); i++)
        {
            const auto currentDeductionResults =
                DeduceTypeArg(argTypes.at(i), paramTypes.at(i));

            deductionResults.insert(
                end(deductionResults),
                begin(currentDeductionResults),
                end  (currentDeductionResults)
            );
        }

        std::for_each(begin(deductionResults), end(deductionResults),
        [&](const TypeArgDeductionResult& deductionResult)
        {
            const auto didVerifyDeductionResult = diagnostics.Collect(VerifyTypeArgDeductionResult(
                srcLocation,
                deductionResult,
                paramToArgMap
            ));
            if (!didVerifyDeductionResult)
            {
                return;
            }

            paramToArgMap[deductionResult.Param] = deductionResult.Arg;
        });

        const auto optArgs = diagnostics.Collect(
            CollectTypeArgsFromMap(srcLocation, paramToArgMap, params)
        );
        if (!optArgs.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ optArgs.value(), std::move(diagnostics) };
    }

    static auto DeduceTypeArgs(
        const SrcLocation& srcLocation,
        IGenericSymbol* const root,
        const std::vector<ITypeSymbol*>& knownArgs,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto params = root->CollectTypeParams();
        if (params.size() == knownArgs.size())
        {
            return Expected{ knownArgs, std::move(diagnostics) };
        }

        if (!optArgTypes.has_value())
        {
            diagnostics.Add(CreateUnableToDeduceTypeArgsError(srcLocation));
            return std::move(diagnostics);
        }

        auto* const callable = dynamic_cast<ICallableSymbol*>(root);
        if (!callable)
        {
            diagnostics.Add(CreateUnableToDeduceTypeArgsError(srcLocation));
            return std::move(diagnostics);
        }

        const auto paramTypes = callable->CollectParamTypes();
        if (paramTypes.empty())
        {
            diagnostics.Add(CreateUnableToDeduceTypeArgsError(srcLocation));
            return std::move(diagnostics);
        }

        const auto optArg = diagnostics.Collect(TypeArgDeductionAlgorithm(
            srcLocation,
            knownArgs,
            params,
            optArgTypes.value(),
            paramTypes
        ));
        if (!optArg.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ optArg.value(), std::move(diagnostics) };
    }
    
    auto Scope::CollectGenericInstance(
        const SrcLocation& srcLocation,
        IGenericSymbol* const root,
        const std::vector<ITypeSymbol*>& knownArgs,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
        const std::optional<ITypeSymbol*>& optSelfType
    ) -> Expected<ISymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<ITypeSymbol*> args{};

        if (!root->GetTypeArgs().empty())
        {
            const auto optArgs = diagnostics.Collect(
                DeduceTypeArgs(srcLocation, root, knownArgs, optArgTypes)
            );
            if (!optArgs.has_value())
            {
                return std::move(diagnostics);
            }

            args = optArgs.value();
        }

        const auto optResolvedInstance =
            ResolveGenericInstance(root, args, optSelfType);
        if (optResolvedInstance.has_value())
        {
            return Expected
            {
                optResolvedInstance.value(),
                std::move(diagnostics),
            };
        }

        auto* const compilation = root->GetCompilation();

        const auto optSymbol = diagnostics.Collect(
            GenericInstantiator::Instantiate(
                srcLocation,
                root,
                InstantiationContext{ args, optSelfType }
            )
        );
        if (!optSymbol.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ optSymbol.value(), std::move(diagnostics) };
    }

    auto Scope::ForceCollectGenericInstance(
        IGenericSymbol* const root,
        const std::vector<ITypeSymbol*>& knownArgs,
        const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
        const std::optional<ITypeSymbol*>& optSelfType
    ) -> ISymbol*
    {
        return DiagnosticBag::CreateNoError().Collect(CollectGenericInstance(
            SrcLocation{ root->GetCompilation() },
            root,
            knownArgs,
            optArgTypes,
            optSelfType
        )).value();
    }

    auto Scope::CollectTypeParams() const -> std::vector<TypeParamTypeSymbol*>
    {
        auto params = CollectSymbols<TypeParamTypeSymbol>();
        std::sort(begin(params), end(params),
        [](TypeParamTypeSymbol* const lhs, TypeParamTypeSymbol* const rhs)
        {
            return lhs->GetIndex() < rhs->GetIndex();
        });

        return params;
    }

    auto Scope::CollectImplOfFor(
        TraitTypeSymbol* const trait,
        ITypeSymbol* const type
    ) -> std::optional<TraitImplSymbol*>
    {
        std::vector scopes{ trait->GetUnaliased()->GetScope() };
        
        const auto typeScope = type->GetUnaliased()->GetScope();
        if (typeScope != scopes.front())
        {
            scopes.push_back(typeScope);
        }

        std::vector<TraitImplSymbol*> impls{};
        std::for_each(begin(scopes), end(scopes),
        [&](const std::shared_ptr<Scope>& scope)
        {
            const auto allImpls = scope->CollectSymbols<TraitImplSymbol>();

            std::copy_if(begin(allImpls), end(allImpls), back_inserter(impls),
            [&](TraitImplSymbol* const impl)
            {
                return
                    DoPlaceholdersOverlap(trait, impl->GetTrait()) &&
                    DoPlaceholdersOverlap(type, impl->GetType());
            });
        });

        return impls.empty() ? std::nullopt : std::optional{ impls.front() };
    }

    auto Scope::CollectImplOfFor(
        PrototypeSymbol* const prototype,
        ITypeSymbol* const type
    ) -> std::optional<FunctionSymbol*>
    {
        const auto optImpl = CollectImplOfFor(
            prototype->GetParentTrait(),
            type
        );
        if (!optImpl.has_value())
        {
            return std::nullopt;
        }

        const auto& symbolMap = optImpl.value()->GetBodyScope()->m_SymbolMap;

        const auto it = symbolMap.find(prototype->GetName().String);
        if (it == end(symbolMap))
        {
            return std::nullopt;
        }

        auto* const firstFunction = 
            dynamic_cast<FunctionSymbol*>(it->second.front().get());
        if (!firstFunction->IsInstance())
        {
            ACE_ASSERT(it->second.size() == 1);
            return firstFunction;
        }

        const auto optFunction = DiagnosticBag::Create().Collect(
            Scope::CollectGenericInstance(
                SrcLocation{ prototype->GetCompilation() },
                firstFunction->GetGenericRoot(),
                prototype->GetTypeArgs(),
                std::nullopt,
                prototype->GetSelfType()
            )
        );
        if (!optFunction.has_value())
        {
            return std::nullopt;
        }

        auto* const castedFunction =
            dynamic_cast<FunctionSymbol*>(optFunction.value());
        ACE_ASSERT(castedFunction);
        return castedFunction;
    }

    static auto CollectConstrainedTraits(
        const std::shared_ptr<const Scope>& scope,
        ITypeSymbol* const type
    ) -> std::vector<TraitTypeSymbol*>
    {
        std::vector<TraitTypeSymbol*> traits{};

        const auto constraints = scope->CollectSymbols<ConstraintSymbol>();

        std::for_each(begin(constraints), end(constraints),
        [&](ConstraintSymbol* const constraint)
        {
            if (type->GetUnaliased() == constraint->GetType()->GetUnaliased())
            {
                traits.insert(
                    end(traits),
                    begin(constraint->GetTraits()),
                    end  (constraint->GetTraits())
                );
            }
        });

        return traits;
    }

    auto Scope::CollectConstrainedTraits(
        ITypeSymbol* const type
    ) const -> std::vector<TraitTypeSymbol*>
    {
        std::vector<TraitTypeSymbol*> traits{};

        for (
            auto optScope = std::optional{ shared_from_this() };
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            const auto scopeTraits =
                Ace::CollectConstrainedTraits(optScope.value(), type);

            traits.insert(end(traits), begin(scopeTraits), end(scopeTraits));
        }

        return traits;
    }

    auto Scope::ResolveSelfType(
        const SrcLocation& srcLocation
    ) const -> Expected<ITypeSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        for (
            auto optScope = std::optional{ shared_from_this() };
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            const auto symbols = optScope.value()->CollectAllSymbols();

            const auto selfTypeIt = std::find_if(begin(symbols), end(symbols),
            [](ISymbol* const symbol)
            {
                if (dynamic_cast<ImplSelfAliasTypeSymbol*>(symbol))
                {
                    return true;
                }

                if (dynamic_cast<TraitSelfTypeSymbol*>(symbol))
                {
                    return true;
                }

                return false;
            });
            if (selfTypeIt != end(symbols))
            {
                auto* const selfType = dynamic_cast<ITypeSymbol*>(*selfTypeIt);
                ACE_ASSERT(selfType);
                return Expected{ selfType, std::move(diagnostics) };
            }
        }

        diagnostics.Add(CreateSelfReferenceInIncorrectContext(srcLocation));
        return std::move(diagnostics);
    }

    auto Scope::ReimportType(ITypeSymbol* const type) -> Diagnosed<ITypeSymbol*>
    {
        return DeclareSymbol(
            std::make_unique<ReimportAliasTypeSymbol>(shared_from_this(), type)
        );
    }

    Scope::Scope(
        Compilation* const compilation
    ) : Scope
        {
            compilation,
            std::string{ AnonymousIdent::Create("global") },
            std::nullopt,
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
        m_OptName{ optName },
        m_OptAnonymousName
        {
            optName.has_value() ?
                std::nullopt :
                std::optional{ std::string{ AnonymousIdent::Create() } }
        },
        m_OptParent{ optParent },
        m_SymbolMap{},
        m_Children{},
        m_GenericInstantiator{ this }
    {
        m_NestLevel = optParent.has_value() ?
            (optParent.value()->GetNestLevel() + 1) :
            0;
    }

    auto Scope::AddChild(
        const std::optional<std::string>& optName
    ) -> std::shared_ptr<Scope>
    {
        std::shared_ptr<Scope> child
        {
            new Scope(m_Compilation, optName, shared_from_this())
        };

        m_Children.push_back(child);
        return child;
    }

    auto Scope::GetDeclaredSymbol(
        const std::string& name,
        const std::vector<ITypeSymbol*>& typeArgs,
        const std::optional<ITypeSymbol*>& optSelfType
    ) const -> std::optional<ISymbol*>
    {
        const auto matchingNameSymbolsIt = m_SymbolMap.find(name);
        if (matchingNameSymbolsIt == end(m_SymbolMap))
        {
            return std::nullopt;
        }

        auto* const firstSymbol = matchingNameSymbolsIt->second.front().get();

        auto* const generic = dynamic_cast<IGenericSymbol*>(firstSymbol);
        if (!generic)
        {
            return firstSymbol;
        }

        return ResolveGenericInstance(
            generic->GetGenericRoot(),
            typeArgs,
            optSelfType
        );
    }

    auto Scope::OnSymbolDeclared(ISymbol* const symbol) -> void
    {
        GenericInstantiator::OnSymbolDeclared(symbol);
    }

    static auto ResolveTypeArgs(
        const SymbolResolutionContext& context
    ) -> Expected<std::vector<ITypeSymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<ITypeSymbol*> args{};
        std::transform(
            begin(context.TypeArgs),
            end  (context.TypeArgs),
            back_inserter(args),
            [](ITypeSymbol* const arg) { return arg; }
        );

        auto* const compilation = context.BeginScope->GetCompilation();

        const auto& argNames = context.NameSection->TypeArgs;
        std::transform(begin(argNames), end(argNames), back_inserter(args),
        [&](const SymbolName& argName)
        {
            const auto optArg = diagnostics.Collect(
                context.BeginScope->ResolveStaticSymbol<ITypeSymbol>(argName)
            );
            return optArg.value_or(compilation->GetErrorSymbols().GetType());
        });

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Expected{ args, std::move(diagnostics) };
    }

    static auto HasSrcLocation(
        const SymbolResolutionContext& context 
    ) -> bool
    {
        return context.SrcLocation.Buffer != nullptr;
    }

    auto Scope::ResolveSymbolInScopes(
        SymbolResolutionContext context 
    ) -> Expected<ISymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (context.TypeArgs.size() < context.SuppliedTypeArgCount)
        {
            const auto optTypeArgs = diagnostics.Collect(
                ResolveTypeArgs(context)
            );
            if (!optTypeArgs.has_value())
            {
                return std::move(diagnostics);
            }

            context.TypeArgs = optTypeArgs.value();
        }

        const auto optMatchingSymbols = diagnostics.Collect(
            CollectMatchingSymbolsInScopes(context)
        );
        if (!optMatchingSymbols.has_value())
        {
            return std::move(diagnostics);
        }

        const auto optSymbol = diagnostics.Collect(context.IsLastNameSection() ? 
            ResolveLastNameSectionSymbol(context, optMatchingSymbols.value()) :
            ResolveNameSectionSymbol    (context, optMatchingSymbols.value())
        );
        if (!optSymbol.has_value())
        {
            return std::move(diagnostics);
        }

        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Expected{ optSymbol.value(), std::move(diagnostics) };
    }

    auto Scope::CollectMatchingSymbolsInScopes(
        const SymbolResolutionContext& context 
    ) -> Expected<std::vector<ISymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<ISymbol*> matchingSymbols{};
        std::for_each(begin(context.Scopes), end(context.Scopes),
        [&](const std::shared_ptr<const Scope>& scope)
        {
            const auto optOptMatchingSymbol = diagnostics.Collect(
                scope->CollectMatchingSymbol(context)
            );
            if (!optOptMatchingSymbol.has_value())
            {
                return;
            }

            const auto& optMatchingSymbol = optOptMatchingSymbol.value();
            if (!optMatchingSymbol.has_value())
            {
                return;
            }

            matchingSymbols.push_back(optMatchingSymbol.value());
        });

        if (matchingSymbols.empty() && diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        return Expected{ matchingSymbols, std::move(diagnostics) };
    }

    auto Scope::CollectMatchingSymbol(
        const SymbolResolutionContext& context 
    ) const -> Expected<std::optional<ISymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto symbolsIt = m_SymbolMap.find(context.GetName());
        if (symbolsIt == end(m_SymbolMap))
        {
            return Expected
            {
                std::optional<ISymbol*>{},
                std::move(diagnostics),
            };
        }

        const auto& symbols = symbolsIt->second;
        ACE_ASSERT(!symbols.empty());

        auto* const symbol = symbols.front().get();

        auto* const generic = dynamic_cast<IGenericSymbol*>(symbol);
        if (!generic || !generic->IsInstance())
        {
            ACE_ASSERT(symbols.size() == 1);
            return Expected{ std::optional{ symbol }, std::move(diagnostics) };
        }

        if (context.IsRoot)
        {
            return Expected
            {
                std::optional<ISymbol*>{ generic->GetGenericRoot() },
                std::move(diagnostics),
            };
        }

        const auto optGenericInstance = diagnostics.Collect(
            CollectGenericInstance(
                context.SrcLocation,
                generic->GetGenericRoot(),
                context.TypeArgs,
                context.OptArgTypes,
                context.OptSelfType
            )
        );
        if (!optGenericInstance.has_value())
        {
            return std::move(diagnostics);
        }

        return Expected{ optGenericInstance.value(), std::move(diagnostics) };
    }

    static auto DoTypeArgsMatch(
        ISymbol* const genericInstance,
        const std::vector<ITypeSymbol*>& typeArgs
    ) -> bool
    {
        auto* const generic = dynamic_cast<IGenericSymbol*>(genericInstance);
        ACE_ASSERT(generic);

        if (typeArgs.size() != generic->GetTypeArgs().size())
        {
            return false;
        }

        for (size_t i = 0; i < typeArgs.size(); i++)
        {
            auto* const lhsType = typeArgs.at(i)->GetUnaliased();
            auto* const rhsType = generic->GetTypeArgs().at(i)->GetUnaliased();

            if (lhsType != rhsType)
            {
                return false;
            }
        }

        return true;
    }

    static auto DoesPrototypeSelfTypeMatch(
        ISymbol* const genericInstance,
        const std::optional<ITypeSymbol*>& optSelfType
    ) -> bool
    {
        auto* const prototype =
            dynamic_cast<PrototypeSymbol*>(genericInstance->GetUnaliased());
        if (!prototype)
        {
            return true;
        }

        if (!optSelfType.has_value())
        {
            return false;
        }

        return
            prototype->GetSelfType()->GetUnaliased() ==
            optSelfType.value()->GetUnaliased();
    }

    auto Scope::ResolveGenericInstance(
        const IGenericSymbol* const root,
        std::vector<ITypeSymbol*> typeArgs,
        const std::optional<ITypeSymbol*>& optSelfType
    ) -> std::optional<ISymbol*>
    {
        const auto& instances =
            root->GetScope()->m_SymbolMap.at(root->GetName().String);

        if (root->GetTypeArgs().empty())
        {
            typeArgs.clear();
        }

        const auto instanceIt = std::find_if(begin(instances), end(instances),
        [&](const std::unique_ptr<ISymbol>& instance)
        {
            return
                DoTypeArgsMatch(instance.get(), typeArgs) &&
                DoesPrototypeSelfTypeMatch(instance.get(), optSelfType);
        });

        return (instanceIt == end(instances)) ?
            std::nullopt :
            std::optional{ instanceIt->get() };
    }

    auto Scope::FindStaticBeginScope(
        const SrcLocation& srcLocation,
        const SymbolName& name
    ) const -> Expected<std::shared_ptr<const Scope>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& globalScope = GetCompilation()->GetGlobalScope();

        const auto& nameString = name.Sections.front().Name.String;

        const bool isNativeType = KeywordToTokenKindMap.contains(nameString);
        const bool isGlobal =
            name.IsGlobal && globalScope->m_SymbolMap.contains(nameString);

        if (isNativeType || isGlobal)
        {
            return Expected{ globalScope, std::move(diagnostics) };
        }

        for (
            auto optScope = std::optional{ shared_from_this() }; 
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            const auto& scope = optScope.value();

            if (scope->m_SymbolMap.contains(nameString))
            {
                return Expected{ scope, std::move(diagnostics) };
            }
        }

        diagnostics.Add(CreateUndeclaredSymbolRefError(srcLocation));
        return std::move(diagnostics);
    }

    static auto GetNameSectionTypeArgs(
        const SymbolResolutionContext& context,
        IAccessibleBodyScopedSymbol* const bodyScoped
    ) -> const std::vector<ITypeSymbol*>&
    {
        auto* const generic = dynamic_cast<IGenericSymbol*>(bodyScoped);
        if (!generic || generic->GetTypeArgs().empty())
        {
            return context.TypeArgs;
        }

        const auto& args = generic->GetTypeArgs();

        return (context.TypeArgs.size() > args.size()) ?
            context.TypeArgs :
            args;
    }

    auto Scope::ResolveNameSectionSymbol(
        const SymbolResolutionContext& context,
        const std::vector<ISymbol*>& matchingSymbols
    ) -> Expected<ISymbol*>
    {
        auto optSpecialSymbol = ResolveSpecialSymbol(context);
        if (optSpecialSymbol.has_value())
        {
            return std::move(optSpecialSymbol.value());
        }

        auto diagnostics = DiagnosticBag::Create();

        if (matchingSymbols.empty())
        {
            diagnostics.Add(CreateUndeclaredSymbolRefError(
                context.SrcLocation
            ));
            return std::move(diagnostics);
        }

        if (matchingSymbols.size() > 1)
        {
            diagnostics.Add(CreateAmbiguousSymbolRefError(
                context.SrcLocation,
                matchingSymbols
            ));
        }

        auto* const symbol = matchingSymbols.front();

        auto* const bodyScoped =
            dynamic_cast<IAccessibleBodyScopedSymbol*>(symbol->GetUnaliased());

        if (!bodyScoped)
        {
            diagnostics.Add(CreateScopeAccessOfNonBodyScopedSymbolError(
                context.SrcLocation,
                symbol
            ));
            return std::move(diagnostics);
        }

        const size_t suppliedTypeArgCount =
            context.SuppliedTypeArgCount +
            (context.NameSection + 1)->TypeArgs.size();

        const auto& typeArgs = GetNameSectionTypeArgs(context, bodyScoped);

        const auto nextNameSection = context.NameSection + 1;

        auto* const type = dynamic_cast<ITypeSymbol*>(bodyScoped);

        if (!type)
        {
            const std::shared_ptr<const Scope> bodyScope =
                bodyScoped->GetBodyScope();

            const auto optSymbol = diagnostics.Collect(ResolveSymbolInScopes({
                nextNameSection->CreateSrcLocation(),
                context.BeginScope,
                context.NameSectionsBegin,
                context.NameSectionsEnd,
                nextNameSection,
                context.OptArgTypes,
                context.IsCorrectSymbolType,
                std::vector{ bodyScope },
                suppliedTypeArgCount,
                typeArgs,
                context.IsRoot,
                std::nullopt,
            }));
            if (!optSymbol.has_value())
            {
                return std::move(diagnostics);
            }

            return Expected{ optSymbol.value(), std::move(diagnostics) };
        }

        std::optional<ISymbol*> optSymbol{};

        const auto inherentScopes =
            CollectInherentScopes(nextNameSection->Name.String, type);

        auto inherentDiagnostics = DiagnosticBag::Create();
        optSymbol = inherentDiagnostics.Collect(ResolveSymbolInScopes({
            nextNameSection->CreateSrcLocation(),
            context.BeginScope,
            context.NameSectionsBegin,
            context.NameSectionsEnd,
            nextNameSection,
            context.OptArgTypes,
            context.IsCorrectSymbolType,
            inherentScopes,
            suppliedTypeArgCount,
            typeArgs,
            context.IsRoot,
            type
        }));   
        
        if (optSymbol.has_value())
        {
            diagnostics.Add(std::move(inherentDiagnostics));
        }
        else
        {
            const auto optTraitScopes = diagnostics.Collect(
                context.BeginScope->CollectTraitScopes(nextNameSection->Name, type)
            );
            if (!optTraitScopes.has_value())
            {
                return std::move(diagnostics);
            }

            auto traitDiagnostics = DiagnosticBag::Create();
            optSymbol = traitDiagnostics.Collect(ResolveSymbolInScopes({
                nextNameSection->CreateSrcLocation(),
                context.BeginScope,
                context.NameSectionsBegin,
                context.NameSectionsEnd,
                nextNameSection,
                context.OptArgTypes,
                context.IsCorrectSymbolType,
                optTraitScopes.value(),
                suppliedTypeArgCount,
                typeArgs,
                context.IsRoot,
                type
            }));
            if (!optSymbol.has_value())
            {
                diagnostics.Add(std::move(traitDiagnostics));
                return std::move(diagnostics);
            }

            diagnostics.Add(std::move(traitDiagnostics));
        }

        return Expected{ optSymbol.value(), std::move(diagnostics) };
    }

    auto Scope::ResolveLastNameSectionSymbol(
        const SymbolResolutionContext& context,
        const std::vector<ISymbol*>& matchingSymbols
    ) -> Expected<ISymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto optExpectedSpecialSymbol = ResolveSpecialSymbol(context);
        if (optExpectedSpecialSymbol.has_value())
        {
            const auto optSpecialSymbol = diagnostics.Collect(
                std::move(optExpectedSpecialSymbol.value())
            );
            if (!optSpecialSymbol.has_value())
            {
                return std::move(diagnostics);
            }

            const bool isCorrectSymbolType = diagnostics.Collect(
                context.IsCorrectSymbolType(
                    context.SrcLocation,
                    optSpecialSymbol.value()
                )
            );
            if (!isCorrectSymbolType)
            {
                return std::move(diagnostics);
            }

            return Expected{ optSpecialSymbol.value(), std::move(diagnostics) };
        }

        if (matchingSymbols.empty())
        {
            diagnostics.Add(CreateUndeclaredSymbolRefError(
                context.SrcLocation
            ));
            return std::move(diagnostics);
        }

        std::vector<ISymbol*> symbols{};
        std::copy_if(
            begin(matchingSymbols), 
            end  (matchingSymbols), 
            back_inserter(symbols), 
            [&](ISymbol* const matchingSymbol) -> bool
            {
                return context.IsCorrectSymbolType(
                    context.SrcLocation,
                    matchingSymbol
                );
            }
        );

        if (symbols.empty())
        {
            const bool isCorrectSymbolType = diagnostics.Collect(
                context.IsCorrectSymbolType(
                    context.SrcLocation,
                    matchingSymbols.front()
                )
            );
            if (!isCorrectSymbolType)
            {
                return std::move(diagnostics);
            }
        }

        if (symbols.size() > 1)
        {
            diagnostics.Add(CreateAmbiguousSymbolRefError(
                context.SrcLocation,
                symbols
            ));
        }

        auto* const symbol = symbols.front();

        if (auto* const prototype = dynamic_cast<PrototypeSymbol*>(symbol))
        {
            if (!prototype->IsDynDispatchable())
            {
                const auto optInstantiatedPrototype = diagnostics.Collect(
                    Scope::CollectGenericInstance(
                        context.SrcLocation,
                        prototype,
                        std::vector<ITypeSymbol*>{},
                        std::nullopt,
                        context.OptSelfType
                    )
                );
                if (!optInstantiatedPrototype.has_value())
                {
                    return std::move(diagnostics);
                }

                return Expected
                {
                    optInstantiatedPrototype.value(),
                    std::move(diagnostics),
                };
            }
        }

        diagnostics.Collect(DiagnoseInaccessibleSymbol(
            context.SrcLocation,
            symbol,
            context.BeginScope
        ));

        return Expected{ symbol, std::move(diagnostics) };
    }

    auto Scope::CollectInherentImplFor(
        const std::string& name,
        ITypeSymbol* type
    ) -> std::optional<InherentImplSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        type = type->GetUnaliasedType();

        auto* const packageMod = type->GetScope()->FindPackageMod();
        const auto packageBodyScope = packageMod->GetBodyScope();

        const auto allImpls =
            packageBodyScope->CollectSymbolsRecursive<InherentImplSymbol>();

        const auto implIt = std::find_if(begin(allImpls), end(allImpls),
        [&](InherentImplSymbol* const impl)
        {
            return
                DoPlaceholdersOverlap(type, impl->GetType()) &&
                impl->GetBodyScope()->HasSymbolWithName(name);
        });

        return (implIt == end(allImpls)) ?
            std::nullopt :
            std::optional{ *implIt };
    }
    
    static auto CollectMatchingNameImplSymbol(
        const std::string& name,
        TraitImplSymbol* const impl
    ) -> ISymbol*
    {
        const auto implSymbols = impl->GetBodyScope()->CollectAllSymbols();

        const auto matchingNameSymbolIt = std::find_if(
            begin(implSymbols),
            end  (implSymbols),
            [&](ISymbol* const symbol)
            {
                return symbol->GetName().String == name;
            }
        );
        ACE_ASSERT(matchingNameSymbolIt != end(implSymbols));
        return *matchingNameSymbolIt;
    }

    static auto CollectMatchingNameImplSymbols(
        const std::string& name,
        const std::vector<TraitImplSymbol*>& impls
    ) -> std::vector<ISymbol*>
    {
        std::vector<ISymbol*> symbols{};
        std::transform(begin(impls), end(impls), back_inserter(symbols),
        [&](TraitImplSymbol* const impl)
        {
            return CollectMatchingNameImplSymbol(name, impl);
        });

        return symbols;
    }

    auto Scope::CollectTraitImplFor(
        const Ident& name,
        ITypeSymbol* type
    ) const -> Expected<std::optional<TraitImplSymbol*>>
    {
        auto diagnostics = DiagnosticBag::Create();

        type = type->GetUnaliasedType();

        const auto modBodyScope = FindMod().value()->GetBodyScope();

        const auto uses   = modBodyScope->CollectSymbols<UseSymbol>();
        const auto traits = modBodyScope->CollectSymbols<TraitTypeSymbol>();

        auto rootTraits = traits;
        std::transform(
            begin(uses),
            end  (uses),
            back_inserter(rootTraits),
            [&](UseSymbol* const use) { return use->GetRootTrait(); }
        );

        std::set<TraitImplSymbol*> implSet{};
        std::for_each(begin(rootTraits), end(rootTraits),
        [&](TraitTypeSymbol* trait)
        {
            if (const auto optImpl = CollectImplOfFor(trait, type))
            {
                implSet.insert(optImpl.value());
            }
        });

        std::vector<TraitImplSymbol*> impls{};
        std::copy_if(begin(implSet), end(implSet), back_inserter(impls),
        [&](TraitImplSymbol* const impl)
        {
            return impl->GetBodyScope()->HasSymbolWithName(name.String);
        });

        if (impls.empty())
        {
            return Expected
            {
                std::optional<TraitImplSymbol*>{},
                std::move(diagnostics),
            };
        }

        if (impls.size() > 1)
        {
            diagnostics.Add(CreateAmbiguousSymbolRefError(
                name.SrcLocation,
                CollectMatchingNameImplSymbols(name.String, impls)
            ));
            return std::move(diagnostics);
        }

        return Expected{ impls.front(), std::move(diagnostics) };
    }

    auto Scope::CollectInherentScopes(
        const std::string& name,
        ITypeSymbol* const type
    ) -> std::vector<std::shared_ptr<const Scope>>
    {
        std::vector<std::shared_ptr<const Scope>> scopes{};
        scopes.push_back(type->GetBodyScope());

        const auto optImpl = CollectInherentImplFor(name, type);
        if (optImpl.has_value())
        {
            scopes.push_back(optImpl.value()->GetBodyScope());
        }

        return scopes;
    }

    auto Scope::CollectTraitScopes(
        const Ident& name,
        ITypeSymbol* const type
    ) const -> Expected<std::vector<std::shared_ptr<const Scope>>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const trait =
            dynamic_cast<TraitTypeSymbol*>(type->GetUnaliased());
        if (trait)
        {
            return Expected
            {
                std::vector<std::shared_ptr<const Scope>>
                {
                    trait->GetPrototypeScope()
                },
                std::move(diagnostics),
            };
        }

        const auto optOptImpl = diagnostics.Collect(
            CollectTraitImplFor(name, type)
        );
        if (!optOptImpl.has_value())
        {
            return std::move(diagnostics);
        }

        if (optOptImpl.value().has_value())
        {
            return Expected
            {
                std::vector<std::shared_ptr<const Scope>>
                {
                    optOptImpl.value().value()->GetBodyScope()
                },
                std::move(diagnostics),
            };
        }

        const auto traits = CollectConstrainedTraits(type);

        std::vector<std::shared_ptr<const Scope>> scopes{};
        std::for_each(begin(traits), end(traits),
        [&](TraitTypeSymbol* const trait)
        {
            if (trait->GetPrototypeScope()->HasSymbolWithName(name.String))
            {
                scopes.push_back(trait->GetPrototypeScope());
            }
        });

        return Expected{ scopes, std::move(diagnostics) };
    }

    auto Scope::ResolveSpecialSymbol(
        const SymbolResolutionContext& context 
    ) -> std::optional<Expected<ISymbol*>>
    {
        const auto tokenKindIt = KeywordToTokenKindMap.find(context.GetName());
        if (tokenKindIt == end(KeywordToTokenKindMap))
        {
            return std::nullopt;
        }

        const auto tokenKind = tokenKindIt->second;
        switch (tokenKind)
        {
            case TokenKind::SelfTypeKeyword:
            {
                return context.BeginScope->ResolveSelfType(context.SrcLocation);
            }

            default:
            {
                auto* const nativeTypeSymbol = GetTokenKindNativeTypeSymbol(
                    context.BeginScope->GetCompilation(),
                    tokenKindIt->second
                );
                return Expected{ nativeTypeSymbol, DiagnosticBag::Create() };
            }
        }
    }
}
