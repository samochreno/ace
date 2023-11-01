#include "Symbols/Impls/InherentImplSymbol.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "AnonymousIdent.hpp"
#include "Noun.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/NominalTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    InherentImplSymbol::InherentImplSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope,
        INominalTypeSymbol* const type
    ) : m_BodyScope{ bodyScope },
        m_Name
        {
            srcLocation,
            AnonymousIdent::Create("impl", type->CreateSignature()),
        },
        m_Type{ type }
    {
    }

    auto InherentImplSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::An, "inherent implementation" };
    }

    auto InherentImplSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto InherentImplSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto InherentImplSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto InherentImplSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto InherentImplSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<InherentImplSymbol>(
            GetName().SrcLocation,
            scope->CreateChild(),
            CreateInstantiated<INominalTypeSymbol>(GetType(), context)
        );
    }

    auto InherentImplSymbol::GetConstrainedScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto InherentImplSymbol::GetType() const -> INominalTypeSymbol*
    {
        return m_Type;
    }
}
