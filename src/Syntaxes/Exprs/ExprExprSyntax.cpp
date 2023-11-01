#include "Syntaxes/Exprs/ExprExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/ExprExprSema.hpp"

namespace Ace
{
    ExprExprSyntax::ExprExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto ExprExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExprExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto ExprExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const ExprExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        return Diagnosed
        {
            std::make_shared<const ExprExprSema>(
                GetSrcLocation(),
                exprSema
            ),
            std::move(diagnostics),
        };
    }

    auto ExprExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
