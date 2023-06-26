#include "Symbols/Vars/StaticVarSymbol.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    StaticVarSymbol::StaticVarSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const AccessModifier& t_accessModifier,
        ITypeSymbol* const t_type
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier },
        m_Type{ t_type }
    {
    }

    auto StaticVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarSymbol::GetName() const -> const std::string&
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
