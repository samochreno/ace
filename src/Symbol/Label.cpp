#include "Symbol/Label.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol
{
    Label::Label(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto Label::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Label::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Label::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Label;
    }

    auto Label::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Label::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }
}
