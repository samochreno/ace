#include "Symbols/Vars/LocalVarSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    LocalVarSymbol::LocalVarSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const Identifier& t_name,
        ITypeSymbol* const t_type
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_Type{ t_type }
    {
    }

    auto LocalVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LocalVarSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto LocalVarSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::LocalVar;
    }

    auto LocalVarSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto LocalVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto LocalVarSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }
}
