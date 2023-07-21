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
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const std::optional<std::shared_ptr<const IExprNode>>& optExpr
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_OptExpr{ optExpr }
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ReturnStmtNode>
    {
        const auto clonedOptExpr = [&]() -> std::optional<std::shared_ptr<const IExprNode>>
        {
            if (!m_OptExpr.has_value())
            {
                return std::nullopt;
            }

            return m_OptExpr.value()->CloneInScopeExpr(scope);
        }();

        return std::make_shared<const ReturnStmtNode>(
            m_SourceLocation,
            scope,
            clonedOptExpr
        );
    }

    auto ReturnStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto ReturnStmtNode::CreateBound() const -> Expected<std::shared_ptr<const ReturnStmtBoundNode>>
    {
        ACE_TRY(boundOptExpr, TransformExpectedOptional(m_OptExpr,
        [](const std::shared_ptr<const IExprNode>& expr)
        {
            return expr->CreateBoundExpr();
        }));

        return std::make_shared<const ReturnStmtBoundNode>(
            GetSourceLocation(),
            GetScope(),
            boundOptExpr
        );
    }

    auto ReturnStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
