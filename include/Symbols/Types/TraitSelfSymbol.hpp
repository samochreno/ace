#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class TraitSelfSymbol : public virtual ITypeSymbol
    {
    public:
        TraitSelfSymbol(const SrcLocation& srcLocation, const std::shared_ptr<Scope>& scope);
        virtual ~TraitSelfSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope, const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto SetBodyScope(const std::shared_ptr<Scope>& scope) -> void final;
        auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& final;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        Ident m_Name{};
    };
}
