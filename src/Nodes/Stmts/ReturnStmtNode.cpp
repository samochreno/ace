#include "Nodes/Stmts/ReturnStmtNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ReturnStmtNode::ReturnStmtNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const std::optional<std::shared_ptr<const IExprNode>>& t_optExpr
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_OptExpr{ t_optExpr }
    {
    }

    auto ReturnStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
            m_SourceLocation,
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

    auto ReturnStmtNode::CreateBound() const -> Expected<std::shared_ptr<const ReturnStmtBoundNode>>
    {
        ACE_TRY(boundOptExpr, TransformExpectedOptional(m_OptExpr,
        [](const std::shared_ptr<const IExprNode>& t_expr)
        {
            return t_expr->CreateBoundExpr();
        }));

        return std::make_shared<const ReturnStmtBoundNode>(
            m_Scope,
            boundOptExpr
        );
    }

    auto ReturnStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
