#include "Syntaxes/Exprs/OrExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/OrExprSema.hpp"

namespace Ace
{
    OrExprSyntax::OrExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& lhsExpr,
        const std::shared_ptr<const IExprSyntax>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto OrExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto OrExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto OrExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr)
            .Build();
    }

    auto OrExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const OrExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto lhsExprSema = diagnostics.Collect(
            m_LHSExpr->CreateExprSema()
        );
        const auto rhsExprSema = diagnostics.Collect(
            m_RHSExpr->CreateExprSema()
        );

        return Diagnosed
        {
            std::make_shared<const OrExprSema>(
                GetSrcLocation(),
                lhsExprSema,
                rhsExprSema
            ),
            std::move(diagnostics),
        };
    }

    auto OrExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
