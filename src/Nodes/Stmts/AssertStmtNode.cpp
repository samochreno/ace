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
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprNode>& condition
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_Condition{ condition }
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const AssertStmtNode>
    {
        return std::make_shared<const AssertStmtNode>(
            m_SourceLocation,
            scope,
            m_Condition->CloneInScopeExpr(scope)
        );
    }

    auto AssertStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto AssertStmtNode::CreateBound() const -> Expected<std::shared_ptr<const AssertStmtBoundNode>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        return std::make_shared<const AssertStmtBoundNode>(
            GetSourceLocation(),
            boundCondition
        );
    }

    auto AssertStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
