#include "BoundNodes/Stmts/Jumps/NormalJumpStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace
{
    NormalJumpStmtBoundNode::NormalJumpStmtBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        LabelSymbol* const labelSymbol
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_LabelSymbol{ labelSymbol }
    {
    }

    auto NormalJumpStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto NormalJumpStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto NormalJumpStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const NormalJumpStmtBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto NormalJumpStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto NormalJumpStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        emitter.GetBlockBuilder().Builder.CreateBr(
            emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol)
        );
    }

    auto NormalJumpStmtBoundNode::GetLabelSymbol() const -> LabelSymbol*
    {
        return m_LabelSymbol;
    }
}
