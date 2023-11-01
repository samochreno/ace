#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class LabelSymbol : public virtual ISymbol
    {
    public:
        LabelSymbol(
            const std::shared_ptr<Scope>& scope,
            const Ident& name
        );
        virtual ~LabelSymbol() = default;

        auto CreateTypeNoun() const -> Noun final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;
        auto GetName() const -> const Ident& final;

        auto CreateInstantiated(
            const std::shared_ptr<Scope>& scope,
            const InstantiationContext& context
        ) const -> std::unique_ptr<ISymbol> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
    };
}
