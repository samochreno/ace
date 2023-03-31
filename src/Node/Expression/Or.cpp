#include "Node/Expression/Or.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Expression//Or.hpp"

namespace Ace::Node::Expression
{
    auto Or::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto Or::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::Or>
    {
        return std::make_unique<const Node::Expression::Or>(
            m_LHSExpression->CloneInScopeExpression(t_scope),
            m_RHSExpression->CloneInScopeExpression(t_scope)
            );
    }

    auto Or::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Or>>
    {
        ACE_TRY(boundLHSExpression, m_LHSExpression->CreateBoundExpression());
        ACE_TRY(boundRHSExpression, m_RHSExpression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Expression::Or>(
            boundLHSExpression,
            boundRHSExpression
            );
    }
}
