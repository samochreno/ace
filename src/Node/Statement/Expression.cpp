#include "Node/Statement/Expression.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Statement/Expression.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    auto Expression::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Expression::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Expression>
    {
        return std::make_unique<const Node::Statement::Expression>(m_Expression->CloneInScopeExpression(t_scope));
    }

    auto Expression::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Expression>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Statement::Expression>(boundExpression);
    }
}
