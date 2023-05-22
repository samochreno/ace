#include "Node/Expression/Box.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "Symbol/Type/Base.hpp"
#include "BoundNode/Expression/Box.hpp"

namespace Ace::Node::Expression
{
    auto Box::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Box::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::Box>
    {
        return std::make_shared<const Node::Expression::Box>(
            m_Expression->CloneInScopeExpression(t_scope)
        );
    }

    auto Box::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Box>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Expression::Box>(boundExpression);
    }
}
