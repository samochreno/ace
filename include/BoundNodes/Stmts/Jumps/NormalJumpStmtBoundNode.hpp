#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/Jumps/JumpStmtBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostic.hpp"
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
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            LabelSymbol* const labelSymbol
        );
        virtual ~NormalJumpStmtBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& emitter) const -> void final;

        auto GetLabelSymbol() const -> LabelSymbol* final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        LabelSymbol* m_LabelSymbol{};
    };
}
