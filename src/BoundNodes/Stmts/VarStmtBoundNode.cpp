#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarRefs/StaticVarRefExprBoundNode.hpp"
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
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        LocalVarSymbol* const symbol,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optAssignedExpr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_OptAssignedExpr{ optAssignedExpr }
    {
    }

    auto VarStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto VarStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto VarStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto VarStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
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
            DiagnosticBag{},
            GetSrcLocation(),
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
            DiagnosticBag{},
            GetSrcLocation(),
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

        const auto varRefExpr = std::make_shared<const StaticVarRefExprBoundNode>(
            DiagnosticBag{},
            m_Symbol->GetName().SrcLocation,
            GetScope(),
            m_Symbol
        );

        // Without type checking and implicit conversions,
        // refs can be initialized too
        const auto assignmentStmt = std::make_shared<const NormalAssignmentStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            varRefExpr,
            m_OptAssignedExpr.value()
        );

        assignmentStmt->Emit(emitter);
    }

    auto VarStmtBoundNode::GetSymbol() const -> LocalVarSymbol*
    {
        return m_Symbol;
    }
}
