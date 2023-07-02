#include "Nodes/Exprs/AndExprNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Exprs/AndExprBoundNode.hpp"

namespace Ace
{
    AndExprNode::AndExprNode(
        const std::shared_ptr<const IExprNode>& t_lhsExpr,
        const std::shared_ptr<const IExprNode>& t_rhsExpr
    ) : m_LHSExpr{ t_lhsExpr },
        m_RHSExpr{ t_rhsExpr }
    {
    }

    auto AndExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto AndExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto AndExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const AndExprNode>
    {
        return std::make_shared<const AndExprNode>(
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope)
        );
    }

    auto AndExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto AndExprNode::CreateBound() const -> Expected<std::shared_ptr<const AndExprBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());
        return std::make_shared<const AndExprBoundNode>(
            boundLHSExpr,
            boundRHSExpr
        );
    }

    auto AndExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
