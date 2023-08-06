#include "Nodes/Exprs/ExprExprNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/ExprExprBoundNode.hpp"

namespace Ace
{
    ExprExprNode::ExprExprNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto ExprExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExprExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprExprNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto ExprExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ExprExprNode>
    {
        return std::make_shared<const ExprExprNode>(
            m_SrcLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto ExprExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto ExprExprNode::CreateBound() const -> Diagnosed<std::shared_ptr<const ExprExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnBoundExpr = m_Expr->CreateBoundExpr();
        diagnostics.Add(dgnBoundExpr);

        return Diagnosed
        {
            std::make_shared<const ExprExprBoundNode>(
                GetSrcLocation(),
                dgnBoundExpr.Unwrap()
            ),
            diagnostics,
        };
    }

    auto ExprExprNode::CreateBoundExpr() const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
