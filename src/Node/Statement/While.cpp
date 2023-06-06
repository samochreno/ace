#include "Node/Statement/While.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Statement/Base.hpp"
#include "Node/Expression/Base.hpp"
#include "Node/Statement/Block.hpp"
#include "BoundNode/Statement/While.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Statement
{
    auto While::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto While::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::While>
    {
        return std::make_shared<const Node::Statement::While>(
            t_scope,
            m_Condition->CloneInScopeExpression(t_scope),
            m_Body->CloneInScope(t_scope)
        );
    }

    auto While::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::While>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpression());
        ACE_TRY(boundBody, m_Body->CreateBound());
        return std::make_shared<const BoundNode::Statement::While>(
            m_Scope,
            boundCondition,
            boundBody
            );
    }
}
