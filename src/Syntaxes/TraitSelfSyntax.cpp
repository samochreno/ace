#include "Syntaxes/TraitSelfSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TraitSelfTypeSymbol.hpp"

namespace Ace
{
    TraitSelfSyntax::TraitSelfSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope }
    {
    }

    auto TraitSelfSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TraitSelfSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto TraitSelfSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto TraitSelfSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TraitSelfSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::BeforeType;
    }

    auto TraitSelfSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed
        {
            std::make_unique<TraitSelfTypeSymbol>(
                GetSrcLocation(),
                GetSymbolScope()
            ),
            DiagnosticBag::Create(),
        };
    }
}
