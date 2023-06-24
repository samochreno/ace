#include "Node/Expr/AddressOf.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "BoundNode/Expr//AddressOf.hpp"

namespace Ace::Node::Expr
{
    auto AddressOf::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto AddressOf::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::AddressOf>
    {
        return std::make_shared<const Node::Expr::AddressOf>(
            m_Expr->CloneInScopeExpr(t_scope)
        );
    }

    auto AddressOf::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::AddressOf>>
    {
        ACE_TRY(boundExpr, m_Expr->CreateBoundExpr());
        return std::make_shared<const BoundNode::Expr::AddressOf>(boundExpr);
    }
}
