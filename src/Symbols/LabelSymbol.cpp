#include "Symbols/LabelSymbol.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Ident.hpp"
#include "Noun.hpp"
#include "AccessModifier.hpp"

namespace Ace
{
    LabelSymbol::LabelSymbol(
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto LabelSymbol::CreateTypeNoun() const -> Noun
    {
        return Noun{ Article::A, "label" };
    }

    auto LabelSymbol::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelSymbol::GetCategory() const -> SymbolCategory
    {
        return SymbolCategory::Static;
    }

    auto LabelSymbol::GetAccessModifier() const -> AccessModifier
    {
        return AccessModifier::Pub;
    }

    auto LabelSymbol::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto LabelSymbol::CreateInstantiated(
        const std::shared_ptr<Scope>& scope,
        const InstantiationContext& context
    ) const -> std::unique_ptr<ISymbol>
    {
        return std::make_unique<LabelSymbol>(scope, GetName());
    }
}
