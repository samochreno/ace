#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ExprDropData.hpp"
#include "CFA.hpp"

namespace Ace
{
    NormalAssignmentStmtBoundNode::NormalAssignmentStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto NormalAssignmentStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalAssignmentStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto NormalAssignmentStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto NormalAssignmentStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const NormalAssignmentStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        auto* const lhsExprTypeSymbol =
            m_LHSExpr->GetTypeInfo().Symbol->GetWithoutRef();

        const auto dgnCheckedLHSExpr = CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            TypeInfo{ lhsExprTypeSymbol, ValueKind::L }
        );
        diagnostics.Add(dgnCheckedLHSExpr);

        const auto dgnCheckedRHSExpr = CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            TypeInfo{ lhsExprTypeSymbol, ValueKind::R }
        );
        diagnostics.Add(dgnCheckedRHSExpr);

        if (
            (dgnCheckedLHSExpr.Unwrap() == m_LHSExpr) &&
            (dgnCheckedRHSExpr.Unwrap() == m_RHSExpr)
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const NormalAssignmentStmtBoundNode>(
                GetSrcLocation(),
                dgnCheckedLHSExpr.Unwrap(),
                dgnCheckedRHSExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    auto NormalAssignmentStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto NormalAssignmentStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const NormalAssignmentStmtBoundNode>
    {
        const auto loweredLHSExpr = m_LHSExpr->CreateLoweredExpr({});
        const auto loweredRHSExpr = m_RHSExpr->CreateLoweredExpr({});

        if (
            (loweredLHSExpr == m_LHSExpr) &&
            (loweredRHSExpr == m_RHSExpr)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const NormalAssignmentStmtBoundNode>(
            GetSrcLocation(),
            loweredLHSExpr,
            loweredRHSExpr
        )->CreateLowered({});
    }

    auto NormalAssignmentStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto NormalAssignmentStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        std::vector<ExprDropData> tmps{};

        const auto rhsEmitResult = m_RHSExpr.get()->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(rhsEmitResult.Tmps),
            end  (rhsEmitResult.Tmps)
        );

        const auto lhsEmitResult = m_LHSExpr.get()->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(lhsEmitResult.Tmps),
            end  (lhsEmitResult.Tmps)
        );

        emitter.EmitCopy(
            lhsEmitResult.Value,
            rhsEmitResult.Value,
            m_LHSExpr->GetTypeInfo().Symbol
        );

        emitter.EmitDropTmps(tmps);
    }

    auto NormalAssignmentStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return {};
    }
}
