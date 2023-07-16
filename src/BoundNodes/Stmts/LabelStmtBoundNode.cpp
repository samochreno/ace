#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace
{
    LabelStmtBoundNode::LabelStmtBoundNode(
        const SourceLocation& t_sourceLocation,
        LabelSymbol* const t_symbol
    ) : m_SourceLocation{ t_sourceLocation },
        m_Symbol{ t_symbol }
    {
    }

    auto LabelStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto LabelStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto LabelStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto LabelStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const LabelStmtBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto LabelStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto LabelStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const LabelStmtBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto LabelStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto LabelStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
    }

    auto LabelStmtBoundNode::GetLabelSymbol() const -> LabelSymbol*
    {
        return m_Symbol;
    }
}
