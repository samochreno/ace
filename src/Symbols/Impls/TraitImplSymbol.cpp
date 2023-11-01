#include "Symbols/Impls/TraitImplSymbol.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "AnonymousIdent.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    TraitImplSymbol::TraitImplSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        TraitTypeSymbol* const trait,
        ITypeSymbol* const type
    ) : m_BodyScope{ bodyScope },
        m_Name
        {
            srcLocation,
            AnonymousIdent::Create(
                "impl",
                trait->CreateSignature(),
                type->CreateSignature()
            ),
        },
        m_Trait{ trait },
        m_Type{ type }
    {
    }

    auto TraitImplSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "trait implementation" };
    }

    auto TraitImplSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto TraitImplSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto TraitImplSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto TraitImplSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto TraitImplSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<TraitImplSymbol>(
            GetName().SrcLocation,
            scope->CreateChild(),
            CreateInstantiated<TraitTypeSymbol>(GetTrait(), context),
            CreateInstantiated<ITypeSymbol>(GetType(), context)
        );
    }

    auto TraitImplSymbol::GetConstrainedScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto TraitImplSymbol::GetTrait() const -> TraitTypeSymbol*
    {
        return m_Trait;
    }

    auto TraitImplSymbol::GetType() const -> ITypeSymbol*
    {
        return m_Type;
    }
}
