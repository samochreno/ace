#pragma once

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
            Scope* const t_scope,
            const std::string& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~Label() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::Label; }
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Public; }

    private:
        Scope* m_Scope{};
        std::string m_Name{};
    };
}
