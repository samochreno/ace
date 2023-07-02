#include "BoundNodes/Stmts/BlockEndStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    BlockEndStmtBoundNode::BlockEndStmtBoundNode(
        const std::shared_ptr<Scope>& t_selfScope
    ) : m_SelfScope{ t_selfScope }
    {
    }

    auto BlockEndStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto BlockEndStmtBoundNode::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto BlockEndStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto BlockEndStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BlockEndStmtBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEndStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto BlockEndStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BlockEndStmtBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEndStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto BlockEndStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
    }
}
