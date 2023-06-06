#include "Node/Expression/Expression.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expression/Expression.hpp"

namespace Ace::Node::Expression
{
    auto Expression::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Expression::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::Expression>
    {
        return std::make_shared<const Node::Expression::Expression>(
            m_Expression->CloneInScopeExpression(t_scope)
        );
    }

    auto Expression::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Expression>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Expression::Expression>(boundExpression);
    }
}
