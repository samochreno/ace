#include "Nodes/Stmts/IfStmtNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"

namespace Ace
{
    IfStmtNode::IfStmtNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const IExprNode>>& t_conditions,
        const std::vector<std::shared_ptr<const BlockStmtNode>>& t_bodies
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Conditions{ t_conditions },
        m_Bodies{ t_bodies }

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
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IfStmtNode>
    {
        std::vector<std::shared_ptr<const IExprNode>> clonedConditions{};
        std::transform(
            begin(m_Conditions),
            end  (m_Conditions),
            back_inserter(clonedConditions),
            [&](const std::shared_ptr<const IExprNode>& t_condition)
            {
                return t_condition->CloneInScopeExpr(t_scope);
            }
        );

        std::vector<std::shared_ptr<const BlockStmtNode>> clonedBodies{};
        std::transform(
            begin(m_Bodies),
            end  (m_Bodies),
            back_inserter(clonedBodies),
            [&](const std::shared_ptr<const BlockStmtNode>& t_body)
            {
                return t_body->CloneInScope(t_scope);
            }
        );

        return std::make_shared<const IfStmtNode>(
            m_SourceLocation,
            t_scope,
            clonedConditions,
            clonedBodies
        );
    }

    auto IfStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto IfStmtNode::CreateBound() const -> Expected<std::shared_ptr<const IfStmtBoundNode>>
    {
        ACE_TRY(boundConditions, TransformExpectedVector(m_Conditions,
        [](const std::shared_ptr<const IExprNode>& t_condition)
        {
            return t_condition->CreateBoundExpr();
        }));

        ACE_TRY(boundBodies, TransformExpectedVector(m_Bodies,
        [](const std::shared_ptr<const BlockStmtNode>& t_body)
        {
            return t_body->CreateBound();
        }));

        return std::make_shared<const IfStmtBoundNode>(
            m_Scope,
            boundConditions,
            boundBodies
        );
    }

    auto IfStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
