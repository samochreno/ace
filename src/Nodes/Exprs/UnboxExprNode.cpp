#include "Nodes/Exprs/UnboxExprNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNode/Expr/Box.hpp"

namespace Ace
{
    UnboxExprNode::UnboxExprNode(
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto UnboxExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UnboxExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UnboxExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const UnboxExprNode>
    {
        return std::make_shared<const UnboxExprNode>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto UnboxExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto UnboxExprNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::Unbox>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::Unbox>(boundExpr);
    }

    auto UnboxExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
