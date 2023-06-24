#include "Symbol/Module.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol
{
    Module::Module(
        const std::shared_ptr<Scope>& t_selfScope,
        const std::string& t_name,
        const AccessModifier& t_accessModifier
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier }
    {
    }

    auto Module::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Module::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto Module::GetName() const -> const std::string&
    {
        return m_Name;
    }

    auto Module::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Module;
    }

    auto Module::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto Module::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }
}
