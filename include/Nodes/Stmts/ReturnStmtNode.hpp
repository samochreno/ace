#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ReturnStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<ReturnStmtNode>,
        public virtual IBindableNode<ReturnStmtBoundNode>
    {
    public:
        ReturnStmtNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            const std::optional<std::shared_ptr<const IExprNode>>& optExpr
        );
        virtual ~ReturnStmtNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const ReturnStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const ReturnStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const IExprNode>> m_OptExpr{};
    };
}
