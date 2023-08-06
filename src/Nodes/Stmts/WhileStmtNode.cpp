#include "Nodes/Stmts/WhileStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNodes/Stmts/WhileStmtBoundNode.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    WhileStmtNode::WhileStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<const IExprNode>& condition,
        const std::shared_ptr<const BlockStmtNode>& body
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Condition{ condition },
        m_Body{ body }
    {
    }

    auto WhileStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto WhileStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto WhileStmtNode::CollectChildren() const -> std::vector<const INode*>
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
            m_SrcLocation,
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

    auto WhileStmtNode::CreateBound() const -> Diagnosed<std::shared_ptr<const WhileStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnBoundCondition = m_Condition->CreateBoundExpr();
        diagnostics.Add(dgnBoundCondition);

        const auto dgnBoundBody = m_Body->CreateBound();
        diagnostics.Add(dgnBoundBody);

        return Diagnosed
        {
            std::make_shared<const WhileStmtBoundNode>(
                GetSrcLocation(),
                GetScope(),
                dgnBoundCondition.Unwrap(),
                dgnBoundBody.Unwrap()
            ),
            diagnostics,
        };
    }

    auto WhileStmtNode::CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
