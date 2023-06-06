#include "Node/Expression/And.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expression//And.hpp"

namespace Ace::Node::Expression
{
    auto And::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto And::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::And>
    {
        return std::make_shared<const Node::Expression::And>(
            m_LHSExpression->CloneInScopeExpression(t_scope),
            m_RHSExpression->CloneInScopeExpression(t_scope)
        );
    }

    auto And::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::And>>
    {
        ACE_TRY(boundLHSExpression, m_LHSExpression->CreateBoundExpression());
        ACE_TRY(boundRHSExpression, m_RHSExpression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Expression::And>(
            boundLHSExpression,
            boundRHSExpression
            );
    }
}
