#pragma once

#include <memory>
#include <string>

#include "Symbol/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol
{
    class Label : public virtual Symbol::IBase
    {
    public:
        Label(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        );
        virtual ~Label() = default;

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
