#pragma once

#include <memory>
#include <vector>

#include "Nodes/Stmts/StmtNode.hpp"
#include "BoundNodes/Stmts/BlockStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class BlockStmtNode :
        public virtual IStmtNode,
        public virtual ICloneableInScopeNode<BlockStmtNode>,
        public virtual IBindableNode<BlockStmtBoundNode>
    {
    public:
        BlockStmtNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& selfScope,
            const std::vector<std::shared_ptr<const IStmtNode>>& stmts
        );
        virtual ~BlockStmtNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const BlockStmtNode> final;
        auto CloneInScopeStmt(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const IStmtNode> final;
        auto CreateBound() const -> std::shared_ptr<const BlockStmtBoundNode> final;
        auto CreateBoundStmt() const -> std::shared_ptr<const IStmtBoundNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_SelfScope{};
        std::vector<std::shared_ptr<const IStmtNode>> m_Stmts{};
    };
}
