#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "BoundNode/Stmt/Block.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class BlockStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableNode<BlockStmtNode>,
        public virtual IBindableNode<BoundNode::Stmt::Block>
    {
    public:
        BlockStmtNode(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::vector<std::shared_ptr<const IStmtNode>>& t_stmts
        );
        virtual ~BlockStmtNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const BlockStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Block>> final;
        auto CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>> final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::vector<std::shared_ptr<const IStmtNode>> m_Stmts{};
    };
}
