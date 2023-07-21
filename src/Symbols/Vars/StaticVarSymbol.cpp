#include "Symbols/Vars/StaticVarSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    StaticVarSymbol::StaticVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Identifier& name,
        const AccessModifier accessModifier,
        ITypeSymbol* const type
    ) : m_Scope{ scope },
        m_Name{ name },
        m_AccessModifier{ accessModifier },
        m_Type{ type }
    {
    }

    auto StaticVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto StaticVarSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::StaticVar;
    }

    auto StaticVarSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto StaticVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto StaticVarSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }
}
