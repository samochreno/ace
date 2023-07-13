#include "Nodes/Stmts/AssertStmtNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/AssertStmtBoundNode.hpp"

namespace Ace
{
    AssertStmtNode::AssertStmtNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const IExprNode>& t_condition
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Condition{ t_condition }
    {
    }

    auto AssertStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
            m_SourceLocation,
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

    auto AssertStmtNode::CreateBound() const -> Expected<std::shared_ptr<const AssertStmtBoundNode>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        return std::make_shared<const AssertStmtBoundNode>(boundCondition);
    }

    auto AssertStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
