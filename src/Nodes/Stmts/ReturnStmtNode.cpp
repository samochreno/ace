#include "Nodes/Stmts/ReturnStmtNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Stmt/Return.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    ReturnStmtNode::ReturnStmtNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::optional<std::shared_ptr<const IExprNode>>& t_optExpr
    ) : m_Scope{ t_scope },
        m_OptExpr{ t_optExpr }
    {
    }

    auto ReturnStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ReturnStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        if (m_OptExpr.has_value())
        {
            AddChildren(children, m_OptExpr.value());
        }

        return children;
    }

    auto ReturnStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const ReturnStmtNode>
    {
        const auto clonedOptExpr = [&]() -> std::optional<std::shared_ptr<const IExprNode>>
        {
            if (!m_OptExpr.has_value())
            {
                return std::nullopt;
            }

            return m_OptExpr.value()->CloneInScopeExpr(t_scope);
        }();

        return std::make_shared<const ReturnStmtNode>(
            t_scope,
            clonedOptExpr
        );
    }

    auto ReturnStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto ReturnStmtNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Return>>
    {
        ACE_TRY(boundOptExpr, TransformExpectedOptional(m_OptExpr,
        [](const std::shared_ptr<const IExprNode>& t_expr)
        {
            return t_expr->CreateBoundExpr();
        }));

        return std::make_shared<const BoundNode::Stmt::Return>(
            m_Scope,
            boundOptExpr
        );
    }

    auto ReturnStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }
}
