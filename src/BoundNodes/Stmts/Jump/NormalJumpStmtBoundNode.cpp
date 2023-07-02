#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace
{
    NormalJumpStmtBoundNode::NormalJumpStmtBoundNode(
        const std::shared_ptr<Scope>& t_scope,
        LabelSymbol* const t_labelSymbol
    ) : m_Scope{ t_scope },
        m_LabelSymbol{ t_labelSymbol }
    {
    }

    auto NormalJumpStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalJumpStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto NormalJumpStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto NormalJumpStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto NormalJumpStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto NormalJumpStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto NormalJumpStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        t_emitter.GetBlockBuilder().Builder.CreateBr(
            t_emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol)
        );
    }

    auto NormalJumpStmtBoundNode::GetLabelSymbol() const -> LabelSymbol*
    {
        return m_LabelSymbol;
    }
}
