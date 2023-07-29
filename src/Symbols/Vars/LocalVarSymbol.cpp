#include "Symbols/Vars/LocalVarSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    LocalVarSymbol::LocalVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        ITypeSymbol* const type
    ) : m_Scope{ scope },
        m_Name{ name },
        m_Type{ type }
    {
    }

    auto LocalVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LocalVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto LocalVarSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::LocalVar;
    }

    auto LocalVarSymbol::GetCategory() const -> SymbolCategory
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
