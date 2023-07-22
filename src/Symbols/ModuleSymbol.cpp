#include "Symbols/ModuleSymbol.hpp"

#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ModuleSymbol::ModuleSymbol(
        const std::shared_ptr<Scope>& selfScope,
        const Ident& name,
        const AccessModifier accessModifier
    ) : m_SelfScope{ selfScope },
        m_Name{ name },
        m_AccessModifier{ accessModifier }
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

    auto ModuleSymbol::GetName() const -> const Ident&
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
