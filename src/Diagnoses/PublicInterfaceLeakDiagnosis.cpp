#include "Diagnoses/PublicInterfaceLeakDiagnosis.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "AccessModifier.hpp"
#include "Compilation.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Scope.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    using VisibilityScopeMap =
        std::map<const ISymbol*, std::shared_ptr<const Scope>>;

    static auto IsAncestorOrSame(
        const std::shared_ptr<const Scope>& ancestor,
        const std::shared_ptr<const Scope>& scope
    ) -> bool
    {
        return
            (ancestor.get() == scope.get()) ||
            ancestor->HasChild(scope);
    }

    static auto SelectNarrowerScope(
        const std::shared_ptr<const Scope>& lhs,
        const std::shared_ptr<const Scope>& rhs
    ) -> std::shared_ptr<const Scope>
    {
        if (IsAncestorOrSame(lhs, rhs))
        {
            return rhs;
        }

        return lhs;
    }

    static auto FindContainingImpls(
        const FunctionSymbol* const function,
        const std::vector<ISymbol*>& symbols
    ) -> std::vector<const ISymbol*>
    {
        std::vector<const ISymbol*> impls{};

        std::for_each(begin(symbols), end(symbols),
        [&](ISymbol* const symbol)
        {
            if (const auto* const impl =
                dynamic_cast<const InherentImplSymbol*>(symbol))
            {
                if (impl->GetBodyScope() == function->GetScope())
                {
                    impls.push_back(impl);
                }
                return;
            }

            if (const auto* const impl =
                dynamic_cast<const TraitImplSymbol*>(symbol))
            {
                if (impl->GetBodyScope() == function->GetScope())
                {
                    impls.push_back(impl);
                }
            }
        });

        return impls;
    }

    static auto GetEffectiveVisibilityScope(
        const ISymbol* const symbol,
        const std::vector<ISymbol*>& symbols,
        VisibilityScopeMap& cache
    ) -> std::shared_ptr<const Scope>
    {
        const auto cachedIt = cache.find(symbol);
        if (cachedIt != end(cache))
        {
            return cachedIt->second;
        }

        const auto symbolScope = symbol->GetScope();
        if (symbol->GetAccessModifier() == AccessModifier::Priv)
        {
            const auto optMod = symbolScope->FindMod();
            const auto visibilityScope = optMod.has_value() ?
                std::shared_ptr<const Scope>{ optMod.value()->GetBodyScope() } :
                std::shared_ptr<const Scope>{ symbolScope };
            cache[symbol] = visibilityScope;
            return visibilityScope;
        }

        auto visibilityScope = std::shared_ptr<const Scope>{ symbolScope };
        const auto optMod = symbolScope->FindMod();
        if (optMod.has_value())
        {
            visibilityScope = GetEffectiveVisibilityScope(
                optMod.value(),
                symbols,
                cache
            );
        }

        if (const auto* const field =
            dynamic_cast<const FieldVarSymbol*>(symbol))
        {
            visibilityScope = SelectNarrowerScope(
                visibilityScope,
                GetEffectiveVisibilityScope(
                    field->GetParentStruct(),
                    symbols,
                    cache
                )
            );
        }

        if (const auto* const prototype =
            dynamic_cast<const PrototypeSymbol*>(symbol))
        {
            visibilityScope = SelectNarrowerScope(
                visibilityScope,
                GetEffectiveVisibilityScope(
                    prototype->GetParentTrait(),
                    symbols,
                    cache
                )
            );
        }

        if (const auto* const function =
            dynamic_cast<const FunctionSymbol*>(symbol))
        {
            const auto impls = FindContainingImpls(function, symbols);
            std::for_each(begin(impls), end(impls),
            [&](const ISymbol* const implSymbol)
            {
                if (const auto* const impl =
                    dynamic_cast<const InherentImplSymbol*>(implSymbol))
                {
                    visibilityScope = SelectNarrowerScope(
                        visibilityScope,
                        GetEffectiveVisibilityScope(
                            impl->GetType(),
                            symbols,
                            cache
                        )
                    );
                    return;
                }

                const auto* const impl =
                    dynamic_cast<const TraitImplSymbol*>(implSymbol);
                visibilityScope = SelectNarrowerScope(
                    visibilityScope,
                    GetEffectiveVisibilityScope(
                        impl->GetType(),
                        symbols,
                        cache
                    )
                );
                visibilityScope = SelectNarrowerScope(
                    visibilityScope,
                    GetEffectiveVisibilityScope(
                        impl->GetTrait(),
                        symbols,
                        cache
                    )
                );
            });
        }

        cache[symbol] = visibilityScope;
        return visibilityScope;
    }

    static auto CollectLeakedTypes(
        ITypeSymbol* const type,
        const std::shared_ptr<const Scope>& interfaceVisibilityScope,
        const std::vector<ISymbol*>& symbols,
        VisibilityScopeMap& visibilityScopeCache,
        std::set<const ITypeSymbol*>& visitedTypes,
        std::set<const ITypeSymbol*>& leakedTypes
    ) -> void
    {
        if (!type || type->IsError())
        {
            return;
        }

        if (
            dynamic_cast<const TypeParamTypeSymbol*>(type) ||
            dynamic_cast<const TraitSelfTypeSymbol*>(type)
        )
        {
            return;
        }

        auto* const unaliasedType = type->GetUnaliasedType();
        if (!visitedTypes.insert(unaliasedType).second)
        {
            return;
        }

        if (
            !dynamic_cast<const TypeParamTypeSymbol*>(unaliasedType) &&
            !dynamic_cast<const TraitSelfTypeSymbol*>(unaliasedType)
        )
        {
            const auto typeVisibilityScope = GetEffectiveVisibilityScope(
                unaliasedType,
                symbols,
                visibilityScopeCache
            );
            if (!IsAncestorOrSame(
                typeVisibilityScope,
                interfaceVisibilityScope
            ))
            {
                leakedTypes.insert(unaliasedType);
            }
        }

        std::for_each(
            begin(unaliasedType->GetTypeArgs()),
            end  (unaliasedType->GetTypeArgs()),
            [&](ITypeSymbol* const typeArg)
        {
            CollectLeakedTypes(
                typeArg,
                interfaceVisibilityScope,
                symbols,
                visibilityScopeCache,
                visitedTypes,
                leakedTypes
            );
        });
    }

    static auto DiagnoseSymbolLeaks(
        ISymbol* const symbol,
        const std::vector<ISymbol*>& symbols,
        VisibilityScopeMap& visibilityScopeCache
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (symbol->GetAccessModifier() != AccessModifier::Pub)
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        std::vector<ITypeSymbol*> interfaceTypes{};
        if (const auto* const callable =
            dynamic_cast<const ICallableSymbol*>(symbol))
        {
            interfaceTypes.push_back(callable->GetType());
            const auto paramTypes = callable->CollectParamTypes();
            interfaceTypes.insert(
                end(interfaceTypes),
                begin(paramTypes),
                end(paramTypes)
            );

            const auto* const constrained =
                dynamic_cast<const IConstrainedSymbol*>(symbol);
            const auto constraints = constrained->CollectConstraints();
            std::for_each(begin(constraints), end(constraints),
            [&](const ConstraintSymbol* const constraint)
            {
                const auto& traits = constraint->GetTraits();
                interfaceTypes.insert(
                    end(interfaceTypes),
                    begin(traits),
                    end(traits)
                );
            });
        }
        else if (const auto* const variable =
            dynamic_cast<const IVarSymbol*>(symbol))
        {
            if (
                dynamic_cast<const GlobalVarSymbol*>(symbol) ||
                dynamic_cast<const FieldVarSymbol*>(symbol)
            )
            {
                interfaceTypes.push_back(variable->GetType());
            }
        }
        else if (const auto* const trait =
            dynamic_cast<const TraitTypeSymbol*>(symbol))
        {
            const auto supertraits = trait->CollectSupertraits();
            std::transform(
                begin(supertraits),
                end(supertraits),
                std::back_inserter(interfaceTypes),
                [](const SupertraitSymbol* const supertrait)
                {
                    return supertrait->GetTrait();
                }
            );
        }

        if (interfaceTypes.empty())
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        const auto interfaceVisibilityScope = GetEffectiveVisibilityScope(
            symbol,
            symbols,
            visibilityScopeCache
        );
        std::set<const ITypeSymbol*> visitedTypes{};
        std::set<const ITypeSymbol*> leakedTypes{};
        std::for_each(begin(interfaceTypes), end(interfaceTypes),
        [&](ITypeSymbol* const interfaceType)
        {
            CollectLeakedTypes(
                interfaceType,
                interfaceVisibilityScope,
                symbols,
                visibilityScopeCache,
                visitedTypes,
                leakedTypes
            );
        });

        std::for_each(begin(leakedTypes), end(leakedTypes),
        [&](const ITypeSymbol* const leakedType)
        {
            diagnostics.Add(CreatePublicInterfaceLeaksPrivateTypeError(
                symbol->GetName().SrcLocation,
                leakedType
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto DiagnosePublicInterfaceLeaks(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();
        const auto symbols = compilation
            ->GetGlobalScope()
            ->CollectAllSymbolsRecursive();
        VisibilityScopeMap visibilityScopeCache{};

        std::for_each(begin(symbols), end(symbols),
        [&](ISymbol* const symbol)
        {
            diagnostics.Collect(DiagnoseSymbolLeaks(
                symbol,
                symbols,
                visibilityScopeCache
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
