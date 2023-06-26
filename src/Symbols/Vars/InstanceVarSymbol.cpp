#include "Symbols/Vars/InstanceVarSymbol.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    InstanceVarSymbol::InstanceVarSymbol(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const AccessModifier& t_accessModifier,
        ITypeSymbol* const t_type,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier },
        m_Type{ t_type },
        m_Index{ t_index }
    {
    }

    auto InstanceVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto InstanceVarSymbol::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto InstanceVarSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::InstanceVar;
    }

    auto InstanceVarSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Instance;
    }

    auto InstanceVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto InstanceVarSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }

    auto InstanceVarSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
