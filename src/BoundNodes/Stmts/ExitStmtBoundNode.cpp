#include "BoundNodes/Stmts/ExitStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace
{
    ExitStmtBoundNode::ExitStmtBoundNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope }
    {
    }

    auto ExitStmtBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto ExitStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ExitStmtBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto ExitStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto ExitStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto ExitStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }
    
    auto ExitStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto ExitStmtBoundNode::Emit(Emitter& t_emitter) const -> void
    {
        auto* const argValue = llvm::ConstantInt::get(
            t_emitter.GetC().GetTypes().GetInt(),
            -1,
            true
        );

        t_emitter.GetBlockBuilder().Builder.CreateCall(
            t_emitter.GetC().GetFunctions().GetExit(),
            { argValue }
        );

        t_emitter.GetBlockBuilder().Builder.CreateUnreachable();
    }
}
