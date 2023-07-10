#include "Nodes/Stmts/WhileStmtNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    WhileStmtNode::WhileStmtNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<const IExprNode>& t_condition,
        const std::shared_ptr<const BlockStmtNode>& t_body
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Condition{ t_condition },
        m_Body{ t_body }
    {
    }

    auto WhileStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto WhileStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto WhileStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Condition);
        AddChildren(children, m_Body);

        return children;
    }

    auto WhileStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const WhileStmtNode>
    {
        return std::make_shared<const WhileStmtNode>(
            m_SourceLocation,
            t_scope,
            m_Condition->CloneInScopeExpr(t_scope),
            m_Body->CloneInScope(t_scope)
        );
    }

    auto WhileStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto WhileStmtNode::CreateBound() const -> Expected<std::shared_ptr<const WhileStmtBoundNode>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        ACE_TRY(boundBody, m_Body->CreateBound());

        return std::make_shared<const WhileStmtBoundNode>(
            m_Scope,
            boundCondition,
            boundBody
        );
    }

    auto WhileStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
