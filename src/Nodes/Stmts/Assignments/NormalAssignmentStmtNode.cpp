#include "Nodes/Stmts/Assignments/NormalAssignmentStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/Assignments/NormalAssignmentStmtBoundNode.hpp"

namespace Ace
{
    NormalAssignmentStmtNode::NormalAssignmentStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprNode>& lhsExpr,
        const std::shared_ptr<const IExprNode>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto NormalAssignmentStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalAssignmentStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalAssignmentStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto NormalAssignmentStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const NormalAssignmentStmtNode>
    {
        return std::make_shared<const NormalAssignmentStmtNode>(
            m_SrcLocation,
            m_Scope,
            m_LHSExpr->CloneInScopeExpr(scope),
            m_RHSExpr->CloneInScopeExpr(scope)
        );
    }

    auto NormalAssignmentStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto NormalAssignmentStmtNode::CreateBound() const -> Expected<std::shared_ptr<const NormalAssignmentStmtBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());
        return std::make_shared<const NormalAssignmentStmtBoundNode>(
            GetSrcLocation(),
            boundLHSExpr,
            boundRHSExpr
        );
    }

    auto NormalAssignmentStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
