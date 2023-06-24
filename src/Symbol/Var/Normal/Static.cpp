#include "Symbol/Var/Normal/Static.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Var::Normal
{
    Static::Static(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const AccessModifier& t_accessModifier,
        Symbol::Type::IBase* const t_type
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier },
        m_Type{ t_type }
    {
    }

    auto Static::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Static::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Static::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::StaticVar;
    }

    auto Static::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Static::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto Static::GetType() const -> Symbol::Type::IBase*
    {
        return m_Type;
    }
}
