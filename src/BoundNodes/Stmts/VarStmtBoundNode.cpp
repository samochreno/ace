#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarReferences/StaticVarReferenceExprBoundNode.hpp"
#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace
{
    VarStmtBoundNode::VarStmtBoundNode(
        const SourceLocation& sourceLocation,
        LocalVarSymbol* const symbol,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optAssignedExpr
    ) : m_SourceLocation{ sourceLocation },
        m_Symbol{ symbol },
        m_OptAssignedExpr{ optAssignedExpr }
    {
    }

    auto VarStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const StmtTypeCheckingContext& context
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
            GetSourceLocation(),
            m_Symbol,
            mchConvertedAndCheckedOptAssignedExpr.Value
        ));
    }

    auto VarStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto VarStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const VarStmtBoundNode>>
    {
        const auto mchLoweredOptAssignedExpr = TransformMaybeChangedOptional(m_OptAssignedExpr,
        [](const std::shared_ptr<const IExprBoundNode>& expr)
        {
            return expr->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredOptAssignedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const VarStmtBoundNode>(
            GetSourceLocation(),
            m_Symbol,
            mchLoweredOptAssignedExpr.Value
        )->GetOrCreateLowered(context).Value);
    }

    auto VarStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto VarStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        if (!m_OptAssignedExpr.has_value())
        {
            return;
        }

        const auto varReferenceExpr = std::make_shared<const StaticVarReferenceExprBoundNode>(
            m_Symbol->GetName().SourceLocation,
            GetScope(),
            m_Symbol
        );

        // Without type checking and implicit conversions,
        // references can be initialized too
        const auto assignmentStmt = std::make_shared<const NormalAssignmentStmtBoundNode>(
            GetSourceLocation(),
            varReferenceExpr,
            m_OptAssignedExpr.value()
        );

        assignmentStmt->Emit(emitter);
    }

    auto VarStmtBoundNode::GetSymbol() const -> LocalVarSymbol*
    {
        return m_Symbol;
    }
}
