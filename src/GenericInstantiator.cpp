#include "GenericInstantiator.hpp"

#include <memory>
#include <vector>
#include <map>

#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/GenericInstantiation.hpp"
#include "FunctionBlockBinding.hpp"
#include "Application.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"
#include "Symbols/ConstraintSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "Symbols/PrototypeSymbol.hpp"

namespace Ace
{
    GenericInstantiator::GenericInstantiator(
        Scope* const scope
    ) : m_Scope{ scope }
    {
    }

    static auto IsSpecialGenericInstance(IGenericSymbol* const root) -> bool
    {
        ACE_ASSERT(root == root->GetRoot());

        const auto& natives = root->GetCompilation()->GetNatives();

        return 
            (root == natives.Ref.TryGetSymbol()) ||
            (root == natives.WeakPtr.TryGetSymbol()) ||
            (root == natives.DynStrongPtr.TryGetSymbol()) ||

            (root == natives.weak_ptr_from.TryGetSymbol()) ||
            (root == natives.weak_ptr_from_dyn.TryGetSymbol()) ||
            (root == natives.weak_ptr_copy.TryGetSymbol()) ||
            (root == natives.weak_ptr_drop.TryGetSymbol()) ||
            (root == natives.weak_ptr_lock.TryGetSymbol()) ||
            (root == natives.weak_ptr_lock_dyn.TryGetSymbol()) ||

            (root == natives.dyn_strong_ptr_from.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_copy.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_drop.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_value_ptr.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_set_value_ptr.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_vtbl_ptr.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_set_vtbl_ptr.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_control_block_ptr.TryGetSymbol()) ||
            (root == natives.dyn_strong_ptr_set_control_block_ptr.TryGetSymbol()) ||

            (root == natives.strong_ptr_to_dyn_strong_ptr.TryGetSymbol());
    }

    static auto DiagnoseUnsizedTypeArgs(
        const SrcLocation& srcLocation,
        IGenericSymbol* const root,
        const std::vector<ITypeSymbol*>& typeArgs
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (IsSpecialGenericInstance(root))
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        std::for_each(begin(typeArgs), end(typeArgs),
        [&](ITypeSymbol* const typeArg)
        {
            if (!dynamic_cast<ISizedTypeSymbol*>(typeArg->GetUnaliased()))
            {
                diagnostics.Add(CreateUnsizedTypeArgError(
                    srcLocation,
                    typeArg
                ));
            }
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto InstantiateScopeSymbol(
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<Scope>& instantiatedScope,
        const InstantiationContext& context,
        ISymbol* const symbol,
        std::map<std::shared_ptr<Scope>, std::shared_ptr<Scope>>& originalToInstantiatedBodyScopeMap
    ) -> void
    {
        if (dynamic_cast<TypeParamTypeSymbol*>(symbol))
        {
            return;
        }
        
        auto ownedInstantiatedSymbol =
            symbol->CreateInstantiated(instantiatedScope, context);

        auto* const instantiatedSymbol = ownedInstantiatedSymbol.get();
        
        (void)DiagnosticBag::CreateNoError().Collect(
            Scope::DeclareSymbol(std::move(ownedInstantiatedSymbol))
        );

        auto* const bodyScopedSymbol = dynamic_cast<IBodyScopedSymbol*>(symbol);
        if (!bodyScopedSymbol)
        {
            return;
        }

        auto* const instantiatedBodyScopedSymbol =
            dynamic_cast<IBodyScopedSymbol*>(instantiatedSymbol);
        ACE_ASSERT(instantiatedBodyScopedSymbol);

        originalToInstantiatedBodyScopeMap[bodyScopedSymbol->GetBodyScope()] =
            instantiatedBodyScopedSymbol->GetBodyScope();
    }

    static auto InstantiateScopeSymbolsAndChildren(
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<Scope>& instantiatedScope,
        const InstantiationContext& context
    ) -> void
    {
        const auto symbols = scope->CollectAllSymbols();

        std::map<std::shared_ptr<Scope>, std::shared_ptr<Scope>> originalToInstantiatedBodyScopeMap{};
        std::for_each(begin(symbols), end(symbols),
        [&](ISymbol* const symbol)
        {
            if (
                dynamic_cast<TypeParamTypeSymbol*>(symbol) ||
                dynamic_cast<ConstraintSymbol*>(symbol)
                )
            {
                return;
            }

            InstantiateScopeSymbol(
                scope,
                instantiatedScope,
                context,
                symbol,
                originalToInstantiatedBodyScopeMap
            );
        });


        const auto childScopes = scope->CollectChildren();
        std::for_each(begin(childScopes), end(childScopes),
        [&](const std::shared_ptr<Scope>& childScope)
        {
            const auto instantiatedChildScope = originalToInstantiatedBodyScopeMap.contains(childScope) ?
                originalToInstantiatedBodyScopeMap.at(childScope) :
                instantiatedScope->CreateChild();

            InstantiateScopeSymbolsAndChildren(
                childScope,
                instantiatedChildScope,
                context
            );
        });
    }

    auto GenericInstantiator::Instantiate(
        const SrcLocation& srcLocation,
        IGenericSymbol* const root,
        const InstantiationContext& context
    ) -> Expected<ISymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto& self = root->GetScope()->GetGenericInstantiator();

        ACE_ASSERT(root == root->GetRoot());
        ACE_ASSERT(root->CollectTypeParams().size() == context.TypeArgs.size());

        diagnostics.Collect(DiagnoseUnsizedTypeArgs(
            srcLocation,
            root,
            context.TypeArgs
        ));
        if (diagnostics.HasErrors())
        {
            return std::move(diagnostics);
        }

        if (auto* const constrained = dynamic_cast<IConstrainedSymbol*>(root))
        {
            diagnostics.Collect(constrained->DiagnoseUnsatisfiedConstraints(
                srcLocation,
                context.TypeArgs
            ));
            if (diagnostics.HasErrors())
            {
                return std::move(diagnostics);
            }
        }

        auto ownedInstance =
            root->CreateInstantiated(root->GetScope(), context);

        auto* const instance =
            dynamic_cast<IGenericSymbol*>(ownedInstance.get());
        ACE_ASSERT(instance);

        self.m_InstanceSet.insert(instance);
        self.m_InstanceToInstantiationSrcLocationMap[instance] = srcLocation;

        (void)DiagnosticBag::CreateNoError().Collect(
            Scope::DeclareSymbol(std::move(ownedInstance))
        );
        DeferOrInstantiateBody(instance);

        return Expected{ instance, std::move(diagnostics) };
    }

    static auto IsInstantiatable(IGenericSymbol* const generic) -> bool
    {
        if (!generic->GetTypeArgs().empty())
        {
            return true;
        }

        auto* const prototype = dynamic_cast<PrototypeSymbol*>(generic);
        return
            prototype &&
            (prototype->GetSelfType() == prototype->CollectSelfType().value());
    }

    auto GenericInstantiator::OnSymbolDeclared(ISymbol* const symbol) -> void
    {
        auto& self = symbol->GetScope()->GetGenericInstantiator();

        if (symbol != symbol->GetUnaliased())
        {
            return;
        }

        auto* const generic = dynamic_cast<IGenericSymbol*>(symbol);
        if (!generic)
        {
            return;
        }
        
        if (!IsInstantiatable(generic))
        {
            return;
        }

        const auto& name = generic->GetName().String;

        if (self.m_NameToRootMap.contains(name))
        {
            return;
        }

        self.m_NameToRootMap[name] = generic;
        self.m_RootToMonosMap[generic] = {};
    }

    auto GenericInstantiator::GetGenericRoot(
        const IGenericSymbol* const instance
    ) -> IGenericSymbol*
    {
        const auto* const unaliasedInstance = instance->GetUnaliased();

        const auto& self =
            unaliasedInstance->GetScope()->GetGenericInstantiator();

        const auto it =
            self.m_NameToRootMap.find(unaliasedInstance->GetName().String);

        ACE_ASSERT(it != end(self.m_NameToRootMap));
        return it->second;
    }


    auto GenericInstantiator::InstantiateBodies(
        const std::vector<FunctionBlockBinding>& functionBlockBindings
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        FinishBodyDeferment();
        diagnostics.Collect(SetupRootBodiesAndMonos(functionBlockBindings));

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto GenericInstantiator::InstantiateReferencedMonos() -> void
    {
        while (true)
        {
            auto instances = CollectAndClearReferencedMonosInstances();
            if (instances.empty())
            {
                break;
            }

            std::for_each(begin(instances), end(instances),
            [&](IGenericSymbol* const instance)
            {
                auto& self = instance->GetScope()->GetGenericInstantiator();

                const auto& monos =
                    self.m_RootToMonosMap.at(instance->GetGenericRoot());

                std::for_each(begin(monos), end(monos),
                [&](IGenericSymbol* const mono)
                {
                    const InstantiationContext context
                    {
                        instance->GetTypeArgs(),
                        std::nullopt,
                    };

                    CreateInstantiated<IGenericSymbol>(mono, context);
                });
            });
        }
    }

    auto GenericInstantiator::IsInstance(const IGenericSymbol* symbol) -> bool
    {
        symbol = dynamic_cast<const IGenericSymbol*>(symbol->GetUnaliased());

        auto& self = symbol->GetScope()->GetGenericInstantiator();

        if (self.m_NameToRootMap.contains(symbol->GetName().String))
        {
            return true;
        }

        return self.m_InstanceSet.contains(symbol);
    }

    auto GenericInstantiator::FinishBodyDeferment() -> void
    {
        while (true)
        {
            auto instances = CollectAndClearDeferredInstances();
            if (instances.empty())
            {
                break;
            }

            std::for_each(begin(instances), end(instances),
            [&](IGenericSymbol* const instance)
            {
                InstantiateBody(instance);
            });
        }

        DisableBodyDeferment();
    }

    auto GenericInstantiator::DisableBodyDeferment() -> void
    {
        m_DoDeferBodyInstantiation = false;

        const auto childScopes = m_Scope->CollectChildren();
        std::for_each(begin(childScopes), end(childScopes),
        [&](const std::shared_ptr<Scope>& childScope)
        {
            childScope->GetGenericInstantiator().DisableBodyDeferment();
        });
    }

    auto GenericInstantiator::DeferOrInstantiateBody(
        IGenericSymbol* const instance
    ) -> void
    {
        auto& self = instance->GetScope()->GetGenericInstantiator();

        if (self.m_DoDeferBodyInstantiation)
        {
            self.m_DeferredInstances.push_back(instance);
        }
        else
        {
            InstantiateBody(instance);
        }
    }

    auto GenericInstantiator::InstantiateBody(
        IGenericSymbol* const instance
    ) -> void
    {
        auto& self = instance->GetScope()->GetGenericInstantiator();

        InstantiateScopeSymbolsAndChildren(
            instance->GetGenericRoot()->GetBodyScope(),
            instance->GetBodyScope(),
            InstantiationContext{ instance->GetTypeArgs(), std::nullopt }
        );

        self.m_ReferencedMonosInstances.emplace_back(instance);
    }

    auto GenericInstantiator::SetupRootBodiesAndMonos(
        const std::vector<FunctionBlockBinding>& functionBlockBindings
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::for_each(begin(functionBlockBindings), end(functionBlockBindings),
        [&](const FunctionBlockBinding& functionBlockBinding)
        {
            auto* const function = functionBlockBinding.Symbol;
            auto& self = function->GetScope()->GetGenericInstantiator();
            
            if (!function->IsPlaceholder())
            {
                return;
            }

            diagnostics.Collect(Application::CreateAndBindFunctionBodies(
                { functionBlockBinding }
            ));

            const auto& optBlockSema = function->GetBlockSema();
            if (optBlockSema.has_value())
            {
                const auto monos = optBlockSema.value()->CollectMonos().Get();
                self.m_RootToMonosMap[function] = std::move(monos);
            }
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto GenericInstantiator::CollectAndClearDeferredInstances() -> std::vector<IGenericSymbol*>
    {
        auto instances = std::move(m_DeferredInstances);
        m_DeferredInstances.clear();

        const auto childScopes = m_Scope->CollectChildren();
        std::for_each(begin(childScopes), end(childScopes),
        [&](const std::shared_ptr<Scope>& childScope)
        {
            const auto childInstances =
                childScope->GetGenericInstantiator().CollectAndClearDeferredInstances();

            instances.insert(
                end(instances),
                begin(childInstances),
                end  (childInstances)
            );
        });

        return instances;
    }

    auto GenericInstantiator::CollectAndClearReferencedMonosInstances() -> std::vector<IGenericSymbol*>
    {
        auto instances = std::move(m_ReferencedMonosInstances);
        m_ReferencedMonosInstances.clear();

        const auto childScopes = m_Scope->CollectChildren();
        std::for_each(begin(childScopes), end(childScopes),
        [&](const std::shared_ptr<Scope>& childScope)
        {
            const auto childInstances =
                childScope->GetGenericInstantiator().CollectAndClearReferencedMonosInstances();

            instances.insert(
                end(instances),
                begin(childInstances),
                end  (childInstances)
            );
        });

        return instances;
    }
}
