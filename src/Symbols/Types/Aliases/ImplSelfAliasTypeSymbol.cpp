#include "Symbols/Types/Aliases/ImplSelfAliasTypeSymbol.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "SpecialIdent.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ImplSelfAliasTypeSymbol::ImplSelfAliasTypeSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const aliasedType
    ) : m_Scope{ scope },
        m_Name{ srcLocation, SpecialIdent::SelfType },
        m_AliasedType{ aliasedType }
    {
    }

    auto ImplSelfAliasTypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "type alias" };
    }

    auto ImplSelfAliasTypeSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplSelfAliasTypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return GetAliasedType()->GetBodyScope();
    }

    auto ImplSelfAliasTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ImplSelfAliasTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto ImplSelfAliasTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto ImplSelfAliasTypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<ImplSelfAliasTypeSymbol>(
            GetName().SrcLocation,
            scope,
            CreateInstantiated<ITypeSymbol>(GetAliasedType(), context)
        );
    }

    auto ImplSelfAliasTypeSymbol::GetAliasedType() const -> ITypeSymbol*
    {
        return m_AliasedType;
    }
}
