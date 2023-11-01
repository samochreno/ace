#include "Symbols/UseSymbol.hpp"

#include <memory>
#include <vector>

#include "Assert.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "AnonymousIdent.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"

namespace Ace
{
    UseSymbol::UseSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        TraitTypeSymbol* const rootTrait
    ) : m_Scope{ scope },
        m_Name{ srcLocation, AnonymousIdent::Create("use") },
        m_RootTrait{ rootTrait }
    {
        ACE_ASSERT(rootTrait == rootTrait->GetRoot());
    }

    auto UseSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "use" };
    }

    auto UseSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto UseSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto UseSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Priv;
    }

    auto UseSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto UseSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<UseSymbol>(
            GetName().SrcLocation,
            scope,
            CreateInstantiated<TraitTypeSymbol>(GetRootTrait(), context)
        );
    }

    auto UseSymbol::GetRootTrait() const -> TraitTypeSymbol*
    {
        return m_RootTrait;
    }
}
