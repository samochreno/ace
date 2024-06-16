#include "Syntaxes/Stmts/Assignments/CompoundAssignmentStmtSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostic.hpp"
#include "Semas/Stmts/Assignments/CompoundAssignmentStmtSema.hpp"
#include "OpResolution.hpp"

namespace Ace
{
    CompoundAssignmentStmtSyntax::CompoundAssignmentStmtSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprSyntax>& lhsExpr,
        const std::shared_ptr<const IExprSyntax>& rhsExpr,
        const SrcLocation& opSrcLocation,
        const Op op
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSrcLocation{ opSrcLocation },
        m_Op{ op }
    {
    }

    auto CompoundAssignmentStmtSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CompoundAssignmentStmtSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto CompoundAssignmentStmtSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr)
            .Build();
    }

    auto CompoundAssignmentStmtSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const CompoundAssignmentStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto lhsExprSema = diagnostics.Collect(
            m_LHSExpr->CreateExprSema()
        );
        const auto rhsExprSema = diagnostics.Collect(
            m_RHSExpr->CreateExprSema()
        );

        const auto opSymbol = diagnostics.Collect(ResolveBinaryOpSymbol(
            m_OpSrcLocation,
            GetScope(),
            lhsExprSema->GetTypeInfo(),
            rhsExprSema->GetTypeInfo(),
            m_Op
        )).value_or(
            GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Diagnosed
        {
            std::make_shared<const CompoundAssignmentStmtSema>(
                GetSrcLocation(),
                lhsExprSema,
                rhsExprSema,
                opSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto CompoundAssignmentStmtSyntax::CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateSema();
    }
}
