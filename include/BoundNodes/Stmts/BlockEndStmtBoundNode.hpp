#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "SourceBuffer.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class BlockEndStmtBoundNode : 
        public std::enable_shared_from_this<BlockEndStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<BlockEndStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<BlockEndStmtBoundNode>
    {
    public:
        BlockEndStmtBoundNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& selfScope
        );
        virtual ~BlockEndStmtBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope>;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BlockEndStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const BlockEndStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_SelfScope{};
    };
}
