#include "Node/Expression/LogicalNegation.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Expression//LogicalNegation.hpp"

namespace Ace::Node::Expression
{
    auto LogicalNegation::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto LogicalNegation::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::LogicalNegation>
    {
        return std::make_shared<const Node::Expression::LogicalNegation>(
            m_Expression->CloneInScopeExpression(t_scope)
        );
    }

    auto LogicalNegation::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::LogicalNegation>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Expression::LogicalNegation>(boundExpression);
    }
}
