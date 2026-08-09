#include "Symbols/Types/TraitSelfSymbol.hpp"

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
    TraitSelfSymbol::TraitSelfSymbol(
        const SrcLocation& srcLocation, const std::shared_ptr<Scope>& scope
    )
        : m_BodyScope{ scope->CreateChild() },
          m_Name{ srcLocation, SpecialIdent::Self }
    {
    }

    auto TraitSelfSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "self type" };
    }

    auto TraitSelfSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto TraitSelfSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TraitSelfSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto TraitSelfSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto TraitSelfSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope, const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<TraitSelfSymbol>(GetName().SrcLocation, scope);
    }

    auto TraitSelfSymbol::SetBodyScope(const std::shared_ptr<Scope>& scope) -> void
    {
        m_BodyScope = scope;
    }

    auto TraitSelfSymbol::GetTypeArgs() const -> const std::vector<ITypeSymbol*>&
    {
        static const std::vector<ITypeSymbol*> args{};
        return args;
    }
}
