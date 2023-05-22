#include "Node/Statement/Assert.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Statement/Assert.hpp"

namespace Ace::Node::Statement
{
    auto Assert::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto Assert::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Assert>
    {
        return std::make_shared<const Node::Statement::Assert>(
            t_scope,
            m_Condition->CloneInScopeExpression(t_scope)
        );
    }

    auto Assert::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Assert>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpression());
        return std::make_shared<const BoundNode::Statement::Assert>(boundCondition);
    }
}
