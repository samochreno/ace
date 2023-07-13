#include "Nodes/Exprs/ExprExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/ExprExprBoundNode.hpp"

namespace Ace
{
    ExprExprNode::ExprExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_SourceLocation{ t_sourceLocation },
        m_Expr{ t_expr }
    {
    }

    auto ExprExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto ExprExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto ExprExprNode::CreateBound() const -> Expected<std::shared_ptr<const ExprExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const ExprExprBoundNode>(boundExpr);
    }

    auto ExprExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
