#include "Symbols/Types/Aliases/ReimportAliasTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ReimportAliasTypeSymbol::ReimportAliasTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const type
    ) : m_Scope{ scope },
        m_Type{ type }
    {
    }

    auto ReimportAliasTypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "reimported type" };
    }

    auto ReimportAliasTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ReimportAliasTypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return GetAliasedType()->GetBodyScope();
    }

    auto ReimportAliasTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ReimportAliasTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto ReimportAliasTypeSymbol::GetName() const -> const Ident&
    {
        return GetAliasedType()->GetName();
    }

    auto ReimportAliasTypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<ReimportAliasTypeSymbol>(
            scope,
            CreateInstantiated<ITypeSymbol>(GetAliasedType(), context)
        );
    }

    auto ReimportAliasTypeSymbol::GetAliasedType() const -> ITypeSymbol*
    {
        return m_Type;
    }
}
