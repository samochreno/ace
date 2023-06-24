#include "Symbol/Variable/Local.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Symbol/Type/Base.hpp"

namespace Ace::Symbol::Variable
{
    Local::Local(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        Symbol::Type::IBase* const t_type
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_Type{ t_type }
    {
    }

    auto Local::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Local::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Local::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::LocalVariable;
    }

    auto Local::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Local::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto Local::GetType() const -> Symbol::Type::IBase*
    {
        return m_Type;
    }
}
