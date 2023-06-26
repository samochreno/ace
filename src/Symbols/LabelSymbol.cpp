#include "Symbols/LabelSymbol.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    LabelSymbol::LabelSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto LabelSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelSymbol::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto LabelSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Label;
    }

    auto LabelSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto LabelSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }
}
