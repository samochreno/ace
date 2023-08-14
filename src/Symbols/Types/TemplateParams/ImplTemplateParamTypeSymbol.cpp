#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ImplTemplateParamTypeSymbol::ImplTemplateParamTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_SelfScope{ scope->GetOrCreateChild({}) },
        m_Name{ name }
    {
    }

    auto ImplTemplateParamTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto ImplTemplateParamTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto ImplTemplateParamTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto ImplTemplateParamTypeSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::ImplTemplateParam;
    }

    auto ImplTemplateParamTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ImplTemplateParamTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }
}
