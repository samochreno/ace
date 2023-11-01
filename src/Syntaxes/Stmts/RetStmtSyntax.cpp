#include "Syntaxes/Stmts/RetStmtSyntax.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/RetStmtSema.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    RetStmtSyntax::RetStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::optional<std::shared_ptr<const IExprSyntax>>& optExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_OptExpr{ optExpr }
    {
    }

    auto RetStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto RetStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto RetStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Collect(m_OptExpr).Build();
    }

    auto RetStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const RetStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::optional<std::shared_ptr<const IExprSema>> optExprSema{};
        if (m_OptExpr.has_value())
        {
            optExprSema =
                diagnostics.Collect(m_OptExpr.value()->CreateExprSema());
        }

        return Diagnosed
        {
            std::make_shared<const RetStmtSema>(
                GetSrcLocation(),
                GetScope(),
                optExprSema
            ),
            std::move(diagnostics),
        };
    }

    auto RetStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
