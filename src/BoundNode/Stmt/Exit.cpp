#include "BoundNode/Stmt/Exit.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Stmt
{
    Exit::Exit(
        const std::shared_ptr<Scope>& t_scope
    ) : m_Scope{ t_scope }
    {
    }

    auto Exit::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Exit::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Exit::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Exit>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Exit::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Exit::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Exit>>
    {
        return CreateUnchanged(shared_from_this());
    }
    
    auto Exit::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Exit::Emit(Emitter& t_emitter) const -> void
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
