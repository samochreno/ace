#include "BoundNode/Statement/Exit.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
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
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Exit>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Exit::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Exit::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Exit>>
    {
        return CreateUnchanged(shared_from_this());
    }
    
    auto Exit::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Exit::Emit(Emitter& t_emitter) const -> void
    {
        auto* const argumentValue = llvm::ConstantInt::get(
            t_emitter.GetC().GetTypes().GetInt(),
            -1,
            true
        );

        t_emitter.GetBlockBuilder().Builder.CreateCall(
            t_emitter.GetC().GetFunctions().GetExit(),
            { argumentValue }
        );

        t_emitter.GetBlockBuilder().Builder.CreateUnreachable();
    }
}
