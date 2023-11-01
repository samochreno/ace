#include "Symbols/ModSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Ident.hpp"
#include "Noun.hpp"

namespace Ace
{
    ModSymbol::ModSymbol(
        const std::shared_ptr<Scope>& bodyScope,
        const AccessModifier accessModifier,
        const Ident& name
    ) : m_BodyScope{ bodyScope },
        m_AccessModifier{ accessModifier },
        m_Name{ name }
    {
    }

    auto ModSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "module" };
    }

    auto ModSymbol::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto ModSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto ModSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto ModSymbol::GetAccessModifier() const -> AccessModifier
    {
        return m_AccessModifier;
    }

    auto ModSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<ModSymbol>(
            scope->CreateChild(),
            GetAccessModifier(),
            GetName()
        );
    }
}
