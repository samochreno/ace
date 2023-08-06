#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExpandableStmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class BlockStmtBoundNode : 
        public std::enable_shared_from_this<BlockStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual IExpandableStmtBoundNode,
        public virtual ITypeCheckableBoundNode<BlockStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<BlockStmtBoundNode>
    {
    public:
        BlockStmtBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& selfScope,
            const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
        );
        virtual ~BlockStmtBoundNode() = default;
        
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const BlockStmtBoundNode>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const BlockStmtBoundNode> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_SelfScope;
        std::vector<std::shared_ptr<const IStmtBoundNode>> m_Stmts{};
    };
}
