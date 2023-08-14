#include "Symbols/Vars/InstanceVarSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    InstanceVarSymbol::InstanceVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const AccessModifier accessModifier,
        ISizedTypeSymbol* const type,
        const size_t index
    ) : m_Scope{ scope },
        m_Name{ name },
        m_AccessModifier{ accessModifier },
        m_Type{ type },
        m_Index{ index }
    {
    }

    auto InstanceVarSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto InstanceVarSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto InstanceVarSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::InstanceVar;
    }

    auto InstanceVarSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Instance;
    }

    auto InstanceVarSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto InstanceVarSymbol::GetType() const -> ISizedTypeSymbol*
    {
        return m_Type;
    }

    auto InstanceVarSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
