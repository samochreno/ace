#include "Symbols/Types/Aliases/TemplateArgs/ImplTemplateArgAliasTypeSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ImplTemplateArgAliasTypeSymbol::ImplTemplateArgAliasTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        ITypeSymbol* const aliasedType,
        const size_t index
    ) : m_Scope{ scope },
        m_Name{ name },
        m_AliasedType{ aliasedType },
        m_Index{ index }
    {
    }

    auto ImplTemplateArgAliasTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AliasedType->GetSelfScope();
    }

    auto ImplTemplateArgAliasTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::TypeAlias;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetAliasedType() const -> ITypeSymbol*
    {
        return m_AliasedType;
    }

    auto ImplTemplateArgAliasTypeSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
