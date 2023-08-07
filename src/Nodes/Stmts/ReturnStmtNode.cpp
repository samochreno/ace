#include "Nodes/Stmts/ReturnStmtNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ReturnStmtNode::ReturnStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::optional<std::shared_ptr<const IExprNode>>& optExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_OptExpr{ optExpr }
    {
    }

    auto ReturnStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ReturnStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ReturnStmtNode::CollectChildren() const -> std::vector<const INode*>
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
            m_SrcLocation,
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

    auto ReturnStmtNode::CreateBound() const -> Diagnosed<std::shared_ptr<const ReturnStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::optional<std::shared_ptr<const IExprBoundNode>> boundOptExpr{};
        if (m_OptExpr.has_value())
        {
            boundOptExpr =
                diagnostics.Collect(m_OptExpr.value()->CreateBoundExpr());
        }

        return Diagnosed
        {
            std::make_shared<const ReturnStmtBoundNode>(
                GetSrcLocation(),
                GetScope(),
                boundOptExpr
            ),
            diagnostics,
        };
    }

    auto ReturnStmtNode::CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }
}
