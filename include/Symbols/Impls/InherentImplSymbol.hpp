#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/BodyScopedSymbol.hpp"
#include "Symbols/ConstrainedSymbol.hpp"
#include "Symbols/Types/NominalTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class InherentImplSymbol :
        public virtual ISymbol,
        public virtual IBodyScopedSymbol,
        public virtual IConstrainedSymbol
    {
    public:
        InherentImplSymbol(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& bodyScope,
            INominalTypeSymbol* const type
        );
        virtual ~InherentImplSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto GetConstrainedScope() const -> std::shared_ptr<Scope> final;

        auto GetType() const -> INominalTypeSymbol*;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        Ident m_Name{};
        INominalTypeSymbol* m_Type{};
    };
}
