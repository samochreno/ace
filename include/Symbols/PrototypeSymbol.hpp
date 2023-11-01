#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/TypedSymbol.hpp"
#include "Symbols/BodyScopedSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "Symbols/ConstrainedSymbol.hpp"
#include "Symbols/CallableSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class TraitTypeSymbol;

    class PrototypeSymbol :
        public virtual ISymbol,
        public virtual ITypedSymbol,
        public virtual IBodyScopedSymbol,
        public virtual IGenericSymbol,
        public virtual IConstrainedSymbol,
        public virtual ICallableSymbol
    {
    public:
        PrototypeSymbol(
            const std::shared_ptr<Scope>& bodyScope,
            const SymbolCategory category,
            const Ident& name,
            const size_t index,
            TraitTypeSymbol* const parentTrait,
            ITypeSymbol* const type,
            ITypeSymbol* const selfType,
            const std::vector<ITypeSymbol*>& typeArgs
        );
        virtual ~PrototypeSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto GetType() const -> ITypeSymbol* final;

        auto SetBodyScope(const std::shared_ptr<Scope>& scope) -> void final;
        auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& final;

        auto GetConstrainedScope() const -> std::shared_ptr<Scope> final;

        auto GetIndex() const -> size_t;
        auto GetParentTrait() const -> TraitTypeSymbol*;
        auto IsDynDispatchable() const -> bool;
        auto GetSelfType() const -> ITypeSymbol*;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        SymbolCategory m_Category{};
        Ident m_Name{};
        size_t m_Index{};
        TraitTypeSymbol* m_ParentTrait{};
        ITypeSymbol* m_Type{};
        ITypeSymbol* m_SelfType{};
        std::vector<ITypeSymbol*> m_TypeArgs{};
    };
}
