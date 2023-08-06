#include "Nodes/Exprs/LogicalNegationExprNode.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

namespace Ace
{
    LogicalNegationExprNode::LogicalNegationExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto LogicalNegationExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LogicalNegationExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto LogicalNegationExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const LogicalNegationExprNode>
    {
        return std::make_shared<const LogicalNegationExprNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto LogicalNegationExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto LogicalNegationExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnBoundExpr = m_Expr->CreateBoundExpr();
        diagnostics.Add(dgnBoundExpr);

        return Diagnosed
        {
            std::make_shared<const LogicalNegationExprBoundNode>(
                GetSrcLocation(),
                dgnBoundExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    auto LogicalNegationExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
