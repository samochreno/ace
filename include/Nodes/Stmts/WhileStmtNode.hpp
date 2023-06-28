#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Stmts/BlockStmtNode.hpp"
#include "BoundNode/Stmt/While.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class WhileStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<WhileStmtNode>,
        public virtual IBindableNode<BoundNode::Stmt::While>
    {
    public:
        WhileStmtNode(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<const IExprNode>& t_condition,
            const std::shared_ptr<const BlockStmtNode>& t_body
        );
        virtual ~WhileStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const WhileStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::While>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprNode> m_Condition{};
        std::shared_ptr<const BlockStmtNode> m_Body{};
    };
}
