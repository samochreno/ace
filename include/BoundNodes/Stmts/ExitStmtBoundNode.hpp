#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class ExitStmtBoundNode : 
        public std::enable_shared_from_this<ExitStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<ExitStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<ExitStmtBoundNode>
    {
    public:
        ExitStmtBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope
        );
        virtual ~ExitStmtBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
    };
}
