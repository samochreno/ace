#include "Syntaxes/Exprs/BoxExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/BoxExprSema.hpp"

namespace Ace
{
    BoxExprSyntax::BoxExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto BoxExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto BoxExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto BoxExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto BoxExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const BoxExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        return Diagnosed
        {
            std::make_shared<const BoxExprSema>(GetSrcLocation(), exprSema),
            std::move(diagnostics),
        };
    }

    auto BoxExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }
}
