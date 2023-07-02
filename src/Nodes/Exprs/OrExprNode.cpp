#include "Nodes/Exprs/OrExprNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Exprs/OrExprBoundNode.hpp"

namespace Ace
{
    OrExprNode::OrExprNode(
        const std::shared_ptr<const IExprNode>& t_lhsExpr,
        const std::shared_ptr<const IExprNode>& t_rhsExpr
    ) : m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr }
    {
    }

    auto OrExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto OrExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto OrExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const OrExprNode>
    {
        return std::make_shared<const OrExprNode>(
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope)
        );
    }

    auto OrExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto OrExprNode::CreateBound() const -> Expected<std::shared_ptr<const OrExprBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());
        return std::make_shared<const OrExprBoundNode>(
            boundLHSExpr,
            boundRHSExpr
        );
    }

    auto OrExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
