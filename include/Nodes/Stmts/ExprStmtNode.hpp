#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/ExprStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class ExprStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<ExprStmtNode>,
        public virtual IBindableNode<ExprStmtBoundNode>
    {
    public:
        ExprStmtNode(const std::shared_ptr<const IExprNode>& t_expr);
        virtual ~ExprStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const ExprStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const ExprStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        std::shared_ptr<const IExprNode> m_Expr{};
    };
}
