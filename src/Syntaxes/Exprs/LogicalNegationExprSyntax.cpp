#include "Syntaxes/Exprs/LogicalNegationExprSyntax.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/LogicalNegationExprSema.hpp"

namespace Ace
{
    LogicalNegationExprSyntax::LogicalNegationExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto LogicalNegationExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LogicalNegationExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto LogicalNegationExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const LogicalNegationExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        return Diagnosed
        {
            std::make_shared<const LogicalNegationExprSema>(
                GetSrcLocation(),
                exprSema
            ),
            std::move(diagnostics),
        };
    }

    auto LogicalNegationExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
