#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    NormalAssignmentStmtBoundNode::NormalAssignmentStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto NormalAssignmentStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
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

    auto NormalAssignmentStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const NormalAssignmentStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const NormalAssignmentStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_LHSExpr,
            m_RHSExpr
        );
    }

    auto NormalAssignmentStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const NormalAssignmentStmtBoundNode>>>
    {
        auto* const lhsExprTypeSymbol =
            m_LHSExpr->GetTypeInfo().Symbol->GetWithoutRef();

        ACE_TRY(cchConvertedAndCheckedLHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpr,
            TypeInfo{ lhsExprTypeSymbol, ValueKind::L }
        ));

        ACE_TRY(cchConvertedAndCheckedRHSExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpr,
            TypeInfo{ lhsExprTypeSymbol, ValueKind::R }
        ));

        if (
            !cchConvertedAndCheckedLHSExpr.IsChanged &&
            !cchConvertedAndCheckedRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const NormalAssignmentStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchConvertedAndCheckedLHSExpr.Value,
            cchConvertedAndCheckedRHSExpr.Value
        ));
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const NormalAssignmentStmtBoundNode>>
    {
        const auto cchLoweredRHSExpr = m_RHSExpr->GetOrCreateLoweredExpr({});
        const auto cchLoweredLHSExpr = m_LHSExpr->GetOrCreateLoweredExpr({});

        if (
            !cchLoweredLHSExpr.IsChanged &&
            !cchLoweredRHSExpr.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const NormalAssignmentStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredLHSExpr.Value,
            cchLoweredRHSExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto NormalAssignmentStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
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
}
