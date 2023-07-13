#include "Nodes/Exprs/AddressOfExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Exprs/AddressOfExprBoundNode.hpp"

namespace Ace
{
    AddressOfExprNode::AddressOfExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto AddressOfExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_Expr->GetSourceLocation();
    }

    auto AddressOfExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOfExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto AddressOfExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const AddressOfExprNode>
    {
        return std::make_shared<const AddressOfExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto AddressOfExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto AddressOfExprNode::CreateBound() const -> Expected<std::shared_ptr<const AddressOfExprBoundNode>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const AddressOfExprBoundNode>(boundExpr);
    }

    auto AddressOfExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
