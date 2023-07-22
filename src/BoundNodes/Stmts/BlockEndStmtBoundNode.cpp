#include "BoundNodes/Stmts/BlockEndStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    BlockEndStmtBoundNode::BlockEndStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& selfScope
    ) : m_SrcLocation{ srcLocation },
        m_SelfScope{ selfScope }
    {
    }

    auto BlockEndStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BlockEndStmtBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEndStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto BlockEndStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const BlockEndStmtBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEndStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto BlockEndStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
    }
}
