#include "Nodes/Stmts/IfStmtNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"

namespace Ace
{
    IfStmtNode::IfStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const IExprNode>>& conditions,
        const std::vector<std::shared_ptr<const BlockStmtNode>>& bodies
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Conditions{ conditions },
        m_Bodies{ bodies }

    {
    }

    auto IfStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto IfStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto IfStmtNode::CollectChildren() const -> std::vector<const INode*>
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
            m_SrcLocation,
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

    auto IfStmtNode::CreateBound() const -> std::shared_ptr<const IfStmtBoundNode>
    {
        std::vector<std::shared_ptr<const IExprBoundNode>> boundConditions{};
        std::transform(
            begin(m_Conditions),
            end  (m_Conditions),
            back_inserter(boundConditions),
            [&](const std::shared_ptr<const IExprNode>& condition)
            {
                return condition->CreateBoundExpr();
            }
        );

        std::vector<std::shared_ptr<const BlockStmtBoundNode>> boundBodies{};
        std::transform(
            begin(m_Bodies),
            end  (m_Bodies),
            back_inserter(boundBodies),
            [&](const std::shared_ptr<const BlockStmtNode>& body)
            {
                return body->CreateBound();
            }
        );

        return std::make_shared<const IfStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            boundConditions,
            boundBodies
        );
    }

    auto IfStmtNode::CreateBoundStmt() const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateBound();
    }
}
