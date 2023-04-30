#include "Node/Statement/If.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Node/Expression/Base.hpp"
#include "Node/Statement/Block.hpp"
#include "Error.hpp"
#include "BoundNode/Statement/If.hpp"

namespace Ace::Node::Statement
{
    auto If::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Conditions);
        AddChildren(children, m_Bodies);

        return children;
    }

    auto If::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::If>
    {
        std::vector<std::shared_ptr<const Node::Expression::IBase>> clonedConditions{};
        std::transform(begin(m_Conditions), end(m_Conditions), back_inserter(clonedConditions),
        [&](const std::shared_ptr<const Node::Expression::IBase>& t_condition)
        {
            return t_condition->CloneInScopeExpression(t_scope);
        });

        std::vector<std::shared_ptr<const Node::Statement::Block>> clonedBodies{};
        std::transform(begin(m_Bodies), end(m_Bodies), back_inserter(clonedBodies),
        [&](const std::shared_ptr<const Node::Statement::Block>& t_body)
        {
            return t_body->CloneInScope(t_scope);
        });

        return std::make_unique<const Node::Statement::If>(
            t_scope,
            clonedConditions,
            clonedBodies
            );
    }

    auto If::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::If>>
    {
        ACE_TRY(boundConditions, TransformExpectedVector(m_Conditions,
        [](const std::shared_ptr<const Node::Expression::IBase>& t_condition)
        {
            return t_condition->CreateBoundExpression();
        }));

        ACE_TRY(boundBodies, TransformExpectedVector(m_Bodies,
        [](const std::shared_ptr<const Node::Statement::Block>& t_body)
        {
            return t_body->CreateBound();
        }));

        return std::make_shared<const BoundNode::Statement::If>(
            m_Scope,
            boundConditions,
            boundBodies
            );
    }
}
