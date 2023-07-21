#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/Jumps/JumpStmtBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
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
            const SourceLocation& sourceLocation,
            const std::shared_ptr<const IExprBoundNode>& condition,
            LabelSymbol* const labelSymbol
        );
        virtual ~ConditionalJumpStmtBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ConditionalJumpStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ConditionalJumpStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto GetLabelSymbol() const -> LabelSymbol* final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const IExprBoundNode> m_Condition{};
        LabelSymbol* m_LabelSymbol{};
    };
}
