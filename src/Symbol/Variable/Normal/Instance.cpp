#include "Symbol/Variable/Normal/Instance.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Symbol/Type/Base.hpp"

namespace Ace::Symbol::Variable::Normal
{
    Instance::Instance(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        const AccessModifier& t_accessModifier,
        Symbol::Type::IBase* const t_type,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier },
        m_Type{ t_type },
        m_Index{ t_index }
    {
    }

    auto Instance::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Instance::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Instance::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::InstanceVariable;
    }

    auto Instance::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Instance;
    }

    auto Instance::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto Instance::GetType() const -> Symbol::Type::IBase*
    {
        return m_Type;
    }

    auto Instance::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
