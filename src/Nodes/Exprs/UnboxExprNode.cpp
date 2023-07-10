#include "Nodes/Exprs/UnboxExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"

namespace Ace
{
    UnboxExprNode::UnboxExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_SourceLocation{ t_sourceLocation },
        m_Expr{ t_expr }
    {
    }

    auto UnboxExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto UnboxExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UnboxExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UnboxExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const UnboxExprNode>
    {
        return std::make_shared<const UnboxExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto UnboxExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto UnboxExprNode::CreateBound() const -> Expected<std::shared_ptr<const UnboxExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const UnboxExprBoundNode>(boundExpr);
    }

    auto UnboxExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
