#include "Node/Expression/AddressOf.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "BoundNode/Expression//AddressOf.hpp"

namespace Ace::Node::Expression
{
    auto AddressOf::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto AddressOf::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::AddressOf>
    {
        return std::make_shared<const Node::Expression::AddressOf>(
            m_Expression->CloneInScopeExpression(t_scope)
        );
    }

    auto AddressOf::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::AddressOf>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Expression::AddressOf>(boundExpression);
    }
}
