#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/VarRefs/StaticVarRefExprBoundNode.hpp"
#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "CFA.hpp"

namespace Ace
{
    VarStmtBoundNode::VarStmtBoundNode(
        const SrcLocation& srcLocation,
        LocalVarSymbol* const symbol,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optAssignedExpr
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_OptAssignedExpr{ optAssignedExpr }
    {
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

    auto VarStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const VarStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::optional<std::shared_ptr<const IExprBoundNode>> checkedOptAssignedExpr{};
        if (m_OptAssignedExpr.has_value())
        {
            checkedOptAssignedExpr = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
                m_OptAssignedExpr.value(),
                TypeInfo{ m_Symbol->GetType(), ValueKind::R }
            ));
        }

        if (checkedOptAssignedExpr == m_OptAssignedExpr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const VarStmtBoundNode>(
                GetSrcLocation(),
                m_Symbol,
                checkedOptAssignedExpr
            ),
            diagnostics,
        };
    }

    auto VarStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto VarStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const VarStmtBoundNode>
    {
        const auto loweredOptAssignedExpr = m_OptAssignedExpr.has_value() ?
            std::optional{ m_OptAssignedExpr.value()->CreateLoweredExpr({}) } :
            std::nullopt;

        if (loweredOptAssignedExpr == m_OptAssignedExpr)
        {
            return shared_from_this();
        }

        return std::make_shared<const VarStmtBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            loweredOptAssignedExpr
        )->CreateLowered({});
    }

    auto VarStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
    }

    auto VarStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        if (!m_OptAssignedExpr.has_value())
        {
            return;
        }

        const auto varRefExpr = std::make_shared<const StaticVarRefExprBoundNode>(
            m_Symbol->GetName().SrcLocation,
            GetScope(),
            m_Symbol
        );

        // Without type checking and implicit conversions,
        // refs can be initialized too
        const auto assignmentStmt = std::make_shared<const NormalAssignmentStmtBoundNode>(
            GetSrcLocation(),
            varRefExpr,
            m_OptAssignedExpr.value()
        );

        assignmentStmt->Emit(emitter);
    }

    auto VarStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return {};
    }

    auto VarStmtBoundNode::GetSymbol() const -> LocalVarSymbol*
    {
        return m_Symbol;
    }
}
