#include "Symbols/Types/Aliases/TemplateArgs/NormalTemplateArgAliasTypeSymbol.hpp"

#include <memory>

#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    NormalTemplateArgAliasTypeSymbol::NormalTemplateArgAliasTypeSymbol(
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

    auto NormalTemplateArgAliasTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AliasedType->GetSelfScope();
    }

    auto NormalTemplateArgAliasTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetKind() const -> SymbolKind
    {
        return SymbolKind::TypeAlias;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Private;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetAliasedType() const -> ITypeSymbol*
    {
        return m_AliasedType;
    }

    auto NormalTemplateArgAliasTypeSymbol::GetIndex() const -> size_t
    {
        return m_Index;
    }
}
