#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarReferences/StaticVarReferenceExprBoundNode.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace
{
    VarStmtBoundNode::VarStmtBoundNode(
        LocalVarSymbol* const t_symbol,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& t_optAssignedExpr
    ) : m_Symbol{ t_symbol },
        m_OptAssignedExpr{ t_optAssignedExpr }
    {
    }

    auto VarStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto VarStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        if (m_OptAssignedExpr.has_value())
        {
            AddChildren(children, m_OptAssignedExpr.value());
        }

        return children;
    }

    auto VarStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const VarStmtBoundNode>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        ACE_TRY(mchConvertedAndCheckedOptAssignedExpr, CreateImplicitlyConvertedAndTypeCheckedOptional(
            m_OptAssignedExpr,
            TypeInfo{ m_Symbol->GetType(), ValueKind::R }
        ));

        if (!mchConvertedAndCheckedOptAssignedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const VarStmtBoundNode>(
            m_Symbol,
            mchConvertedAndCheckedOptAssignedExpr.Value
        ));
    }

    auto VarStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto VarStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const VarStmtBoundNode>>
    {
        const auto mchLoweredOptAssignedExpr = TransformMaybeChangedOptional(m_OptAssignedExpr,
        [](const std::shared_ptr<const IExprBoundNode>& t_expr)
        {
            return t_expr->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredOptAssignedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const VarStmtBoundNode>(
            m_Symbol,
            mchLoweredOptAssignedExpr.Value
        )->GetOrCreateLowered(t_context).Value);
    }

    auto VarStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto VarStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        if (!m_OptAssignedExpr.has_value())
        {
            return;
        }

        const auto variableReferenceExpr = std::make_shared<const StaticVarReferenceExprBoundNode>(
            GetScope(),
            m_Symbol
        );

        // Without type checking and implicit conversions, references can be initialized too.
        const auto assignmentStmt = std::make_shared<const AssignmentStmtBoundNode>(
            variableReferenceExpr,
            m_OptAssignedExpr.value()
        );

        assignmentStmt->Emit(t_emitter);
    }

    auto VarStmtBoundNode::GetSymbol() const -> LocalVarSymbol*
    {
        return m_Symbol;
    }
}
