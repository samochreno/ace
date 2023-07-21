#include "Symbols/LabelSymbol.hpp"

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    LabelSymbol::LabelSymbol(
        const std::shared_ptr<Scope>& scope,
        const Identifier& name
    ) : m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto LabelSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelSymbol::GetName() const -> const Identifier&
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
