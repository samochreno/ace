#include "Nodes/Exprs/ExprExprNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/Expr.hpp"

namespace Ace
{
    ExprExprNode::ExprExprNode(
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto ExprExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ExprExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const ExprExprNode>
    {
        return std::make_shared<const ExprExprNode>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto ExprExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto ExprExprNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Expr>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::Expr>(boundExpr);
    }

    auto ExprExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
