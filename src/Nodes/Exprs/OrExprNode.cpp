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

    auto OrExprNode::CollectChildren() const -> std::vector<const INode*>
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

    auto OrExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const OrExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnBoundLHSExpr = m_LHSExpr->CreateBoundExpr();
        diagnostics.Add(dgnBoundLHSExpr);

        const auto dgnBoundRHSExpr = m_RHSExpr->CreateBoundExpr();
        diagnostics.Add(dgnBoundRHSExpr);

        return Diagnosed
        {
            std::make_shared<const OrExprBoundNode>(
                GetSrcLocation(),
                dgnBoundLHSExpr.Unwrap(),
                dgnBoundRHSExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    auto OrExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
