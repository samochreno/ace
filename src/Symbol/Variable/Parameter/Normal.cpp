#include "Symbol/Variable/Parameter/Normal.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "Symbol/Type/Base.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Variable::Parameter
{
    Normal::Normal(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name,
        Symbol::Type::IBase* const t_type,
        const size_t& t_index
    ) : m_Scope{ t_scope },
        m_Name{ t_name },
        m_Type{ t_type },
        m_Index{ t_index }
    {
    }

    auto Normal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Normal::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ParameterVariable;
    }

    auto Normal::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Normal::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto Normal::GetType() const -> Symbol::Type::IBase*
    {
        return m_Type;
    }

    auto Normal::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
