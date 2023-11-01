#pragma once

#include <memory>
#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class VoidTypeSymbol : public ITypeSymbol
    {
    public:
        VoidTypeSymbol(const std::shared_ptr<Scope>& scope);
        virtual ~VoidTypeSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetBodyScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

        auto SetBodyScope(const std::shared_ptr<Scope>& scope) -> void final;
        auto GetTypeArgs() const -> const std::vector<ITypeSymbol*>& final;

    private:
        std::shared_ptr<Scope> m_BodyScope{};
        Ident m_Name{};
    };
}
