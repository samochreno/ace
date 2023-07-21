#include "Nodes/Exprs/BoxExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "BoundNodes/Exprs/BoxExprBoundNode.hpp"

namespace Ace
{
    BoxExprNode::BoxExprNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprNode>& expr
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr }
    {
    }

    auto BoxExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto BoxExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto BoxExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto BoxExprNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const BoxExprNode>
    {
        return std::make_shared<const BoxExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(scope)
        );
    }

    auto BoxExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto BoxExprNode::CreateBound() const -> Expected<std::shared_ptr<const BoxExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoxExprBoundNode>(
            GetSourceLocation(),
            boundExpr
        );
    }

    auto BoxExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
