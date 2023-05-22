#include "Node/Statement/Assignment/Normal.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Statement/Assignment/Normal.hpp"

namespace Ace::Node::Statement::Assignment
{
    auto Normal::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto Normal::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Assignment::Normal>
    {
        return std::make_shared<const Node::Statement::Assignment::Normal>(
            m_Scope,
            m_LHSExpression->CloneInScopeExpression(t_scope),
            m_RHSExpression->CloneInScopeExpression(t_scope)
        );
    }

    auto Normal::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Assignment::Normal>>
    {
        ACE_TRY(boundLHSExpression, m_LHSExpression->CreateBoundExpression());
        ACE_TRY(boundRHSExpression, m_RHSExpression->CreateBoundExpression());
        return std::make_shared<const BoundNode::Statement::Assignment::Normal>(
            boundLHSExpression,
            boundRHSExpression
        );
    }
}
