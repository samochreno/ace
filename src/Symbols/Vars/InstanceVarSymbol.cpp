#include "Symbols/Vars/InstanceVarSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    InstanceVarSymbol::InstanceVarSymbol(
        const std::shared_ptr<Scope>& scope,
        const Identifier& name,
        const AccessModifier accessModifier,
        ITypeSymbol* const type,
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

    auto InstanceVarSymbol::GetName() const -> const Identifier&
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
