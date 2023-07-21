#pragma once

#include <memory>

#include "Symbols/Symbol.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class LabelSymbol : public virtual ISymbol
    {
    public:
        LabelSymbol(
            const std::shared_ptr<Scope>& scope,
            const Identifier& name
        );
        virtual ~LabelSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Identifier& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Identifier m_Name{};
    };
}
