#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/TypedSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class TraitTypeSymbol;

    class ConstraintSymbol :
        public virtual ISymbol,
        public virtual ITypedSymbol
    {
    public:
        ConstraintSymbol(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            ITypeSymbol* const type,
            const std::vector<TraitTypeSymbol*>& traits
        );
        virtual ~ConstraintSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto GetType() const -> ITypeSymbol* final;
        auto GetTraits() const -> const std::vector<TraitTypeSymbol*>&;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        ITypeSymbol* m_Type{};
        std::vector<TraitTypeSymbol*> m_Traits{};
    };
}
