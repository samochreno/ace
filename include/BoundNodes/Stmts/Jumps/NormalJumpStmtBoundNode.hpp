#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/Jumps/JumpStmtBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class NormalJumpStmtBoundNode :
        public std::enable_shared_from_this<NormalJumpStmtBoundNode>,
        public virtual IJumpStmtBoundNode,
        public virtual ITypeCheckableBoundNode<NormalJumpStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<NormalJumpStmtBoundNode>
    {
    public:
        NormalJumpStmtBoundNode(
            const std::shared_ptr<Scope>& t_scope,
            LabelSymbol* const t_labelSymbol
        );
        virtual ~NormalJumpStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> LabelSymbol* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        LabelSymbol* m_LabelSymbol{};
    };
}
