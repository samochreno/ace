#include "Nodes/Stmts/ExprStmtNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    ExprStmtNode::ExprStmtNode(
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto ExprStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ExprStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const ExprStmtNode>
    {
        return std::make_shared<const ExprStmtNode>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto ExprStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto ExprStmtNode::CreateBound() const -> Expected<std::shared_ptr<const ExprStmtBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const ExprStmtBoundNode>(boundExpr);
    }

    auto ExprStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}