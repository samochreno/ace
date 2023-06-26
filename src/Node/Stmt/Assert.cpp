#include "Node/Stmt/Assert.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Assert.hpp"

namespace Ace::Node::Stmt
{
    Assert::Assert(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const Node::Expr::IBase>& t_condition
    ) : m_Scope{ t_scope },
        m_Condition{ t_condition }
    {
    }

    auto Assert::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Assert::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto Assert::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::Assert>
    {
        return std::make_shared<const Node::Stmt::Assert>(
            t_scope,
            m_Condition->CloneInScopeExpr(t_scope)
        );
    }

    auto Assert::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Assert::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assert>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        return std::make_shared<const BoundNode::Stmt::Assert>(boundCondition);
    }

    auto Assert::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
