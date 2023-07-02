#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/Jumps/JumpStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class ConditionalJumpStmtBoundNode :
        public std::enable_shared_from_this<ConditionalJumpStmtBoundNode>,
        public virtual IJumpStmtBoundNode,
        public virtual ITypeCheckableBoundNode<ConditionalJumpStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<ConditionalJumpStmtBoundNode>
    {
    public:
        ConditionalJumpStmtBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_condition,
            LabelSymbol* const t_labelSymbol
        );
        virtual ~ConditionalJumpStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ConditionalJumpStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ConditionalJumpStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> LabelSymbol* final;

    private:
        std::shared_ptr<const IExprBoundNode> m_Condition{};
        LabelSymbol* m_LabelSymbol{};
    };
}
