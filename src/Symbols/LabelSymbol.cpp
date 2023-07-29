#include "Symbols/LabelSymbol.hpp"

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    LabelSymbol::LabelSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto LabelSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto LabelSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::Label;
    }

    auto LabelSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto LabelSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }
}
