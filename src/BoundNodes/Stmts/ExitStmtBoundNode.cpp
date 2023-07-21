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
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope }
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
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto ExitStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto ExitStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ExitStmtBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }
    
    auto ExitStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto ExitStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        auto* const argValue = llvm::ConstantInt::get(
            emitter.GetC().GetTypes().GetInt(),
            -1,
            true
        );

        emitter.GetBlockBuilder().Builder.CreateCall(
            emitter.GetC().GetFunctions().GetExit(),
            { argValue }
        );

        emitter.GetBlockBuilder().Builder.CreateUnreachable();
    }
}
