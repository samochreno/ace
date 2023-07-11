#include "Symbols/ModuleSymbol.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ModuleSymbol::ModuleSymbol(
        const std::shared_ptr<Scope>& t_selfScope,
        const Identifier& t_name,
        const AccessModifier t_accessModifier
    ) : m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier }
    {
    }

    auto ModuleSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto ModuleSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto ModuleSymbol::GetName() const -> const Identifier&
    {
        return m_Name;
    }

    auto ModuleSymbol::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Module;
    }

    auto ModuleSymbol::GetSymbolCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ModuleSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }
}
