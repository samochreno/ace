#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    NormalTemplateParamTypeSymbol::NormalTemplateParamTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_SelfScope{ scope->GetOrCreateChild({}) },
        m_Name{ name }
    {
    }

    auto NormalTemplateParamTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto NormalTemplateParamTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto NormalTemplateParamTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto NormalTemplateParamTypeSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::TemplateParam;
    }

    auto NormalTemplateParamTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto NormalTemplateParamTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }
}
