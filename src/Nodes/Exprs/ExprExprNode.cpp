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
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr }
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ExprExprNode>
    {
        return std::make_shared<const ExprExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto ExprExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto ExprExprNode::CreateBound() const -> Expected<std::shared_ptr<const ExprExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const ExprExprBoundNode>(
            GetSourceLocation(),
            boundExpr
        );
    }

    auto ExprExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
