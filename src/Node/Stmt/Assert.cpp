#include "Node/Stmt/Assert.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Assert.hpp"

namespace Ace::Node::Stmt
{
    auto Assert::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto Assert::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Assert>
    {
        return std::make_shared<const Node::Stmt::Assert>(
            t_scope,
            m_Condition->CloneInScopeExpr(t_scope)
        );
    }

    auto Assert::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assert>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        return std::make_shared<const BoundNode::Stmt::Assert>(boundCondition);
    }
}
