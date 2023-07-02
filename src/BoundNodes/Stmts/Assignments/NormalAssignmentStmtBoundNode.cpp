#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    AssignmentStmtBoundNode::AssignmentStmtBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& t_rhsExpr
    ) : m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr }
    {
    }

    auto AssignmentStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto AssignmentStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto AssignmentStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const AssignmentStmtBoundNode>>>
    {
        auto* const lhsExprTypeSymbol =
            m_LHSExpr->GetTypeInfo().Symbol->GetWithoutReference();

        ACE_TRY(mchConvertedAndCheckedLHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            TypeInfo{ lhsExprTypeSymbol, ValueKind::L }
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            TypeInfo{ lhsExprTypeSymbol, ValueKind::R }
        ));

        if (
            !mchConvertedAndCheckedLHSExpr.IsChanged &&
            !mchConvertedAndCheckedRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const AssignmentStmtBoundNode>(
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value
        );
        return CreateChanged(returnValue);
    }

    auto AssignmentStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto AssignmentStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const AssignmentStmtBoundNode>>
    {
        const auto mchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});
        const auto mchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});

        if (
            !mchLoweredLHSExpr.IsChanged &&
            !mchLoweredRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const AssignmentStmtBoundNode>(
            mchLoweredLHSExpr.Value,
            mchLoweredRHSExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto AssignmentStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto AssignmentStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        std::vector<ExprDropData> temporaries{};

        const auto rhsEmitResult = m_RHSExpr.get()->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(rhsEmitResult.Temporaries),
            end  (rhsEmitResult.Temporaries)
        );

        const auto lhsEmitResult = m_LHSExpr.get()->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(lhsEmitResult.Temporaries),
            end  (lhsEmitResult.Temporaries)
        );

        t_emitter.EmitCopy(
            lhsEmitResult.Value,
            rhsEmitResult.Value,
            m_LHSExpr->GetTypeInfo().Symbol
        );

        t_emitter.EmitDropTemporaries(temporaries);
    }
}
