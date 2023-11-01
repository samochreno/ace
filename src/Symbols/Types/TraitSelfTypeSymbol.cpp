#include "Symbols/Types/TraitSelfTypeSymbol.hpp"

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
    TraitSelfTypeSymbol::TraitSelfTypeSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_BodyScope{ scope->CreateChild() },
        m_Name{ srcLocation, SpecialIdent::SelfType }
    {
    }

    auto TraitSelfTypeSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "self type" };
    }

    auto TraitSelfTypeSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto TraitSelfTypeSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TraitSelfTypeSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto TraitSelfTypeSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto TraitSelfTypeSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<TraitSelfTypeSymbol>(
            GetName().SrcLocation,
            scope
        );
    }

    auto TraitSelfTypeSymbol::SetBodyScope(
        const std::shared_ptr<Scope>& scope
    ) -> void
    {
        m_BodyScope = scope;
    }

    auto TraitSelfTypeSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        static const std::vector<ITypeSymbol*> args{};
        return args;
    }
}
