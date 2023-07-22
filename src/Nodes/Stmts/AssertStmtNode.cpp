#include "Nodes/Stmts/AssertStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/AssertStmtBoundNode.hpp"

namespace Ace
{
    AssertStmtNode::AssertStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprNode>& condition
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Condition{ condition }
    {
    }

    auto AssertStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
            m_SrcLocation,
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
            GetSrcLocation(),
            boundCondition
        );
    }

    auto AssertStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
