#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNodes/Stmts/IfStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class IfStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<IfStmtNode>,
        public virtual IBindableNode<IfStmtBoundNode>
    {
    public:
        IfStmtNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::vector<std::shared_ptr<const IExprNode>>& t_conditions,
            const std::vector<std::shared_ptr<const BlockStmtNode>>& t_bodies
        );
        virtual ~IfStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IfStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const IfStmtBoundNode>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const IExprNode>> m_Conditions{};
        std::vector<std::shared_ptr<const BlockStmtNode>> m_Bodies{};
    };
}
