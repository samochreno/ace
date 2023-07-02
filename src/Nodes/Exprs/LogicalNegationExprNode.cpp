#include "Nodes/Exprs/LogicalNegationExprNode.hpp"

#include <memory>
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

namespace Ace
{
    LogicalNegationExprNode::LogicalNegationExprNode(
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto LogicalNegationExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto LogicalNegationExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const LogicalNegationExprNode>
    {
        return std::make_shared<const LogicalNegationExprNode>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto LogicalNegationExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto LogicalNegationExprNode::CreateBound() const -> Expected<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());

        return std::make_shared<const LogicalNegationExprBoundNode>(
            boundExpr
        );
    }

    auto LogicalNegationExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
