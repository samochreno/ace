#include "Nodes/Stmts/WhileStmtNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    WhileStmtNode::WhileStmtNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprNode>& condition,
        const std::shared_ptr<const BlockStmtNode>& body
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_Condition{ condition },
        m_Body{ body }
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const WhileStmtNode>
    {
        return std::make_shared<const WhileStmtNode>(
            m_SourceLocation,
            scope,
            m_Condition->CloneInScopeExpr(scope),
            m_Body->CloneInScope(scope)
        );
    }

    auto WhileStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto WhileStmtNode::CreateBound() const -> Expected<std::shared_ptr<const WhileStmtBoundNode>>
    {
        ACE_TRY(boundCondition, m_Condition->CreateBoundExpr());
        ACE_TRY(boundBody, m_Body->CreateBound());
        return std::make_shared<const WhileStmtBoundNode>(
            GetSourceLocation(),
            GetScope(),
            boundCondition,
            boundBody
        );
    }

    auto WhileStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
