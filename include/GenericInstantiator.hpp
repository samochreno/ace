#pragma once

#include <vector>
#include <map>
#include <set>
#include <unordered_map>

#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "FunctionBlockBinding.hpp"

namespace Ace
{
    class Scope;
    class ISymbol;
    class ITypeSymbol;
    class IGenericSymbol;
    class InstantiationContext;

    class GenericInstantiator
    {
    public:
        GenericInstantiator(Scope* const scope);
        ~GenericInstantiator() = default;

        static auto Instantiate(
            const SrcLocation& srcLocation,
            IGenericSymbol* const root,
            const InstantiationContext& context
        ) -> Expected<ISymbol*>;
        static auto OnSymbolDeclared(ISymbol* const symbol) -> void;
        static auto GetGenericRoot(
            const IGenericSymbol* const instance
        ) -> IGenericSymbol*;

        auto InstantiateBodies(
            const std::vector<FunctionBlockBinding>& functionBlockBindings
        ) -> Diagnosed<void>;
        auto InstantiateReferencedMonos() -> void;
        auto FinishInstantiation() -> Diagnosed<void>;

        static auto IsInstance(const IGenericSymbol* const symbol) -> bool;

    private:
        auto FinishBodyDeferment()  -> void;
        auto DisableBodyDeferment() -> void;
        static auto DeferOrInstantiateBody(
            IGenericSymbol* const instance
        ) -> void;
        static auto InstantiateBody(IGenericSymbol* const instance) -> void;

        static auto SetupRootBodiesAndMonos(
            const std::vector<FunctionBlockBinding>& functionBlockBindings
        ) -> Diagnosed<void>;

        auto CollectAndClearDeferredInstances()        -> std::vector<IGenericSymbol*>;
        auto CollectAndClearReferencedMonosInstances() -> std::vector<IGenericSymbol*>;

        Scope* m_Scope{};

        std::unordered_map<std::string, IGenericSymbol*>        m_NameToRootMap{};
        std::map<IGenericSymbol*, std::vector<IGenericSymbol*>> m_RootToMonosMap{};

        bool m_DoDeferBodyInstantiation = true;
        std::set<const IGenericSymbol*> m_InstanceSet{};
        std::vector<IGenericSymbol*> m_DeferredInstances{};
        std::vector<IGenericSymbol*> m_ReferencedMonosInstances{};

        std::map<IGenericSymbol*, SrcLocation> m_InstanceToInstantiationSrcLocationMap{};
    };
}
