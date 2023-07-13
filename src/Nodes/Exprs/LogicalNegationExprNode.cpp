#include "Nodes/Exprs/LogicalNegationExprNode.hpp"

#include <memory>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

namespace Ace
{
    LogicalNegationExprNode::LogicalNegationExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_SourceLocation{ t_sourceLocation },
        m_Expr{ t_expr }
    {
    }

    auto LogicalNegationExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
            m_SourceLocation,
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
