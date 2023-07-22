#include "Nodes/Exprs/OrExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/OrExprBoundNode.hpp"

namespace Ace
{
    OrExprNode::OrExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& lhsExpr,
        const std::shared_ptr<const IExprNode>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto OrExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const OrExprNode>
    {
        return std::make_shared<const OrExprNode>(
            m_SrcLocation,
            m_LHSExpr->CloneInScopeExpr(scope),
            m_RHSExpr->CloneInScopeExpr(scope)
        );
    }

    auto OrExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto OrExprNode::CreateBound() const -> Expected<std::shared_ptr<const OrExprBoundNode>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());
        return std::make_shared<const OrExprBoundNode>(
            GetSrcLocation(),
            boundLHSExpr,
            boundRHSExpr
        );
    }

    auto OrExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
