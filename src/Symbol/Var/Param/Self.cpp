#include "Symbol/Var/Param/Self.hpp"

#include <memory>
#include <string>

#include "Scope.hpp"
#include "Symbol/Type/Base.hpp"
#include "SpecialIdentifier.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Var::Param
{
    Self::Self(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* const t_type
    ) : m_Scope{ t_scope },
        m_Type{ t_type }
    {
    }

    auto Self::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Self::GetName() const -> const std::string&
    {
        static std::string name = SpecialIdentifier::Self;
        return name;
    }

    auto Self::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ParamVar;
    }

    auto Self::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Self::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Public;
    }

    auto Self::GetType() const -> Symbol::Type::IBase*
    {
        return m_Type;
    }
}
