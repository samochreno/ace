#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "BoundNodes/Stmts/ExpandableStmtBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class BlockStmtBoundNode : 
        public std::enable_shared_from_this<BlockStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<BlockStmtBoundNode>,
        public virtual IExpandableStmtBoundNode,
        public virtual ITypeCheckableBoundNode<BlockStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<BlockStmtBoundNode>
    {
    public:
        BlockStmtBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& selfScope,
            const std::vector<std::shared_ptr<const IStmtBoundNode>>& stmts
        );
        virtual ~BlockStmtBoundNode() = default;
        
        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const BlockStmtBoundNode> final;
        auto CloneWithDiagnosticsStmt(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const IStmtBoundNode> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BlockStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const BlockStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtBoundNode>> final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_SelfScope;
        std::vector<std::shared_ptr<const IStmtBoundNode>> m_Stmts{};
    };
}
