#include "Syntaxes/Stmts/Assignments/SimpleAssignmentStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/Assignments/SimpleAssignmentStmtSema.hpp"

namespace Ace
{
    SimpleAssignmentStmtSyntax::SimpleAssignmentStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprSyntax>& lhsExpr,
        const std::shared_ptr<const IExprSyntax>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto SimpleAssignmentStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SimpleAssignmentStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SimpleAssignmentStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr)
            .Build();
    }

    auto SimpleAssignmentStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const SimpleAssignmentStmtSema>>
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
            std::make_shared<const SimpleAssignmentStmtSema>(
                GetSrcLocation(),
                lhsExprSema,
                rhsExprSema
            ),
            std::move(diagnostics),
        };
    }

    auto SimpleAssignmentStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
