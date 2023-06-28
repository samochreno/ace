#include "Nodes/Stmts/AssertStmtNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Assert.hpp"

namespace Ace
{
    AssertStmtNode::AssertStmtNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const IExprNode>& t_condition
    ) : m_Scope{ t_scope },
        m_Condition{ t_condition }
    {
    }

    auto AssertStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto AssertStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto AssertStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const AssertStmtNode>
    {
        return std::make_shared<const AssertStmtNode>(
            t_scope,
            m_Condition->CloneInScopeExpr(t_scope)
        );
    }

    auto AssertStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto AssertStmtNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assert>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        return std::make_shared<const BoundNode::Stmt::Assert>(boundCondition);
    }

    auto AssertStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
