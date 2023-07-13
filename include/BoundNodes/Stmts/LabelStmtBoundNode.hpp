#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Stmts/StmtBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class LabelStmtBoundNode :
        public std::enable_shared_from_this<LabelStmtBoundNode>,
        public virtual IStmtBoundNode,
        public virtual ITypeCheckableBoundNode<LabelStmtBoundNode, StmtTypeCheckingContext>,
        public virtual ILowerableBoundNode<LabelStmtBoundNode>
    {
    public:
        LabelStmtBoundNode(LabelSymbol* const t_symbol);
        virtual ~LabelStmtBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const LabelStmtBoundNode>>> final;
        auto GetOrCreateTypeCheckedStmt(
            const StmtTypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const LabelStmtBoundNode>> final;
        auto GetOrCreateLoweredStmt(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> void final;

        auto GetLabelSymbol() const -> LabelSymbol*;

    private:
        LabelSymbol* m_Symbol{};
    };
}
