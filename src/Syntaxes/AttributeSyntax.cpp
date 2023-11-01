#include "Syntaxes/AttributeSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    AttributeSyntax::AttributeSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const StructConstructionExprSyntax>& structConstructionExpr
    ) : m_SrcLocation{ srcLocation },
        m_StructConstructionExpr{ structConstructionExpr }
    {
    }

    auto AttributeSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AttributeSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto AttributeSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_StructConstructionExpr).Build();
    }
}
