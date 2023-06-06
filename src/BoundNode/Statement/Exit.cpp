#include "BoundNode/Statement/Exit.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    auto Exit::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Exit::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Exit>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Exit::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Exit>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Exit::Emit(Emitter& t_emitter) const -> void
    {
        t_emitter.GetBlockBuilder().Builder.CreateCall(
            t_emitter.GetC().GetFunctions().GetExit(),
            { llvm::ConstantInt::get(t_emitter.GetC().GetTypes().GetInt(), -1, true) }
        );

        t_emitter.GetBlockBuilder().Builder.CreateUnreachable();
    }
}
