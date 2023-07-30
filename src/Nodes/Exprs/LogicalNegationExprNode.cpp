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

    auto LogicalNegationExprNode::CreateBound() const -> Expected<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const LogicalNegationExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            boundExpr
        );
    }

    auto LogicalNegationExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
