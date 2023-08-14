#include "Symbols/Vars/StaticVarSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    StaticVarSymbol::StaticVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const AccessModifier accessModifier,
        ISizedTypeSymbol* const type
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

    auto StaticVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto StaticVarSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::StaticVar;
    }

    auto StaticVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto StaticVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto StaticVarSymbol::GetType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }
}
