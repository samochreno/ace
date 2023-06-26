#include "Node/Expr/AddressOf.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "BoundNode/Expr/AddressOf.hpp"

namespace Ace::Node::Expr
{
    AddressOf::AddressOf(
        const std::shared_ptr<const Node::Expr::IBase>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto AddressOf::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOf::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto AddressOf::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::AddressOf>
    {
        return std::make_shared<const Node::Expr::AddressOf>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto AddressOf::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto AddressOf::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::AddressOf>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::AddressOf>(boundExpr);
    }

    auto AddressOf::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
