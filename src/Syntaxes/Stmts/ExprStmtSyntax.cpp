#include "Syntaxes/Stmts/ExprStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Semas/Stmts/ExprStmtSema.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ExprStmtSyntax::ExprStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSyntax>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto ExprStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExprStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_Expr).Build();
    }

    auto ExprStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const ExprStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto exprSema = diagnostics.Collect(m_Expr->CreateExprSema());

        return Diagnosed
        {
            std::make_shared<const ExprStmtSema>(GetSrcLocation(), exprSema),
            std::move(diagnostics),
        };
    }

    auto ExprStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
