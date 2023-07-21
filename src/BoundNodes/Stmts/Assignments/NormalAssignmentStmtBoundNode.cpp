#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    NormalAssignmentStmtBoundNode::NormalAssignmentStmtBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr
    ) : m_SourceLocation{ sourceLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto NormalAssignmentStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto NormalAssignmentStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto NormalAssignmentStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const NormalAssignmentStmtBoundNode>>>
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

        return CreateChanged(std::make_shared<const NormalAssignmentStmtBoundNode>(
            GetSourceLocation(),
            mchConvertedAndCheckedLHSExpr.Value,
            mchConvertedAndCheckedRHSExpr.Value
        ));
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const NormalAssignmentStmtBoundNode>>
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

        return CreateChanged(std::make_shared<const NormalAssignmentStmtBoundNode>(
            GetSourceLocation(),
            mchLoweredLHSExpr.Value,
            mchLoweredRHSExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto NormalAssignmentStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        std::vector<ExprDropData> temporaries{};

        const auto rhsEmitResult = m_RHSExpr.get()->Emit(emitter);
        temporaries.insert(
            end(temporaries),
            begin(rhsEmitResult.Temporaries),
            end  (rhsEmitResult.Temporaries)
        );

        const auto lhsEmitResult = m_LHSExpr.get()->Emit(emitter);
        temporaries.insert(
            end(temporaries),
            begin(lhsEmitResult.Temporaries),
            end  (lhsEmitResult.Temporaries)
        );

        emitter.EmitCopy(
            lhsEmitResult.Value,
            rhsEmitResult.Value,
            m_LHSExpr->GetTypeInfo().Symbol
        );

        emitter.EmitDropTemporaries(temporaries);
    }
}
