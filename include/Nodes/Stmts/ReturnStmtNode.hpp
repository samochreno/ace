#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNode/Stmt/Return.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class ReturnStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<ReturnStmtNode>,
        public virtual IBindableNode<BoundNode::Stmt::Return>
    {
    public:
        ReturnStmtNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::optional<std::shared_ptr<const IExprNode>>& t_optExpr
        );
        virtual ~ReturnStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const ReturnStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Return>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const IExprNode>> m_OptExpr{};
    };
}
