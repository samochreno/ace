#pragma once

#include <memory>
#include <string>

#include "Symbols/Symbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    class LabelSymbol : public virtual ISymbol
    {
    public:
        LabelSymbol(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        );
        virtual ~LabelSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}
