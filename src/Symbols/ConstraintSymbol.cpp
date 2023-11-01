#include "Symbols/ConstraintSymbol.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Noun.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "AnonymousIdent.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    ConstraintSymbol::ConstraintSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const type,
        const std::vector<TraitTypeSymbol*>& traits
    ) : m_Scope{ scope },
        m_Name{ srcLocation, AnonymousIdent::Create("constraint") },
        m_Type{ type },
        m_Traits{ traits }
    {
    }

    auto ConstraintSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "constraint" };
    }

    auto ConstraintSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ConstraintSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ConstraintSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto ConstraintSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }
    
    auto ConstraintSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        ACE_UNREACHABLE();
    }

    auto ConstraintSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }

    auto ConstraintSymbol::GetTraits() const -> const std::vector<TraitTypeSymbol*>&
    {
        return m_Traits;
    }
}
