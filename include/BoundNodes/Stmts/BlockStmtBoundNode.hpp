#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExpandableStmtBoundNode.hpp"
#include "SourceBuffer.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

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
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_selfScope,
            const std::vector<std::shared_ptr<const IStmtBoundNode>>& t_stmts
        );
        virtual ~BlockStmtBoundNode() = default;
        
        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BlockStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BlockStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_SelfScope;
        std::vector<std::shared_ptr<const IStmtBoundNode>> m_Stmts{};
    };
}
