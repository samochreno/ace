#include "Node/Expression/Unbox.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "Symbol/Type/Base.hpp"
#include "BoundNode/Expression/Box.hpp"

namespace Ace::Node::Expression
{
    auto Unbox::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Unbox::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::Unbox>
    {
        return std::make_shared<const Node::Expression::Unbox>(
            m_Expression->CloneInScopeExpression(t_scope)
        );
    }

    auto Unbox::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::Unbox>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Expression::Unbox>(boundExpression);
    }
}
