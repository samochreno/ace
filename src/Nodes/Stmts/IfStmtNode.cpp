#include "Nodes/Stmts/IfStmtNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"

namespace Ace
{
    IfStmtNode::IfStmtNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IExprNode>>& conditions,
        const std::vector<std::shared_ptr<const BlockStmtNode>>& bodies
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_Conditions{ conditions },
        m_Bodies{ bodies }

    {
    }

    auto IfStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto IfStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto IfStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Conditions);
        AddChildren(children, m_Bodies);

        return children;
    }

    auto IfStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IfStmtNode>
    {
        std::vector<std::shared_ptr<const IExprNode>> clonedConditions{};
        std::transform(
            begin(m_Conditions),
            end  (m_Conditions),
            back_inserter(clonedConditions),
            [&](const std::shared_ptr<const IExprNode>& condition)
            {
                return condition->CloneInScopeExpr(scope);
            }
        );

        std::vector<std::shared_ptr<const BlockStmtNode>> clonedBodies{};
        std::transform(
            begin(m_Bodies),
            end  (m_Bodies),
            back_inserter(clonedBodies),
            [&](const std::shared_ptr<const BlockStmtNode>& body)
            {
                return body->CloneInScope(scope);
            }
        );

        return std::make_shared<const IfStmtNode>(
            m_SourceLocation,
            scope,
            clonedConditions,
            clonedBodies
        );
    }

    auto IfStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto IfStmtNode::CreateBound() const -> Expected<std::shared_ptr<const IfStmtBoundNode>>
    {
        ACE_TRY(boundConditions, TransformExpectedVector(m_Conditions,
        [](const std::shared_ptr<const IExprNode>& condition)
        {
            return condition->CreateBoundExpr();
        }));

        ACE_TRY(boundBodies, TransformExpectedVector(m_Bodies,
        [](const std::shared_ptr<const BlockStmtNode>& body)
        {
            return body->CreateBound();
        }));

        return std::make_shared<const IfStmtBoundNode>(
            GetSourceLocation(),
            GetScope(),
            boundConditions,
            boundBodies
        );
    }

    auto IfStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
