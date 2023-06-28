#include "Nodes/Stmts/AssignmentStmtNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Assignment/Normal.hpp"

namespace Ace
{
    AssignmentStmtNode::AssignmentStmtNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const IExprNode>& t_lhsExpr,
        const std::shared_ptr<const IExprNode>& t_rhsExpr
    ) : m_Scope{ t_scope },
        m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr }
    {
    }

    auto AssignmentStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto AssignmentStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto AssignmentStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const AssignmentStmtNode>
    {
        return std::make_shared<const AssignmentStmtNode>(
            m_Scope,
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope)
        );
    }

    auto AssignmentStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto AssignmentStmtNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assignment::Normal>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());

        return std::make_shared<const BoundNode::Stmt::Assignment::Normal>(
            boundLHSExpr,
            boundRHSExpr
        );
    }

    auto AssignmentStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
