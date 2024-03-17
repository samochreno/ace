#include "Symbols/SupertraitSymbol.hpp"

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "AnonymousIdent.hpp"

namespace Ace
{
    SupertraitSymbol::SupertraitSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        TraitTypeSymbol* const trait
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ std::move(scope) },
        m_Name{ srcLocation, AnonymousIdent::Create("supertrait") },
        m_Trait{ trait }
    {
    }

    auto SupertraitSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "supertrait" };
    }

    auto SupertraitSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SupertraitSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto SupertraitSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto SupertraitSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto SupertraitSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<SupertraitSymbol>(
            GetName().SrcLocation,
            GetScope(),
            CreateInstantiated<TraitTypeSymbol>(GetTrait(), context)
        );
    }

    auto SupertraitSymbol::GetTrait() const -> TraitTypeSymbol*
    {
        return m_Trait;
    }
}
