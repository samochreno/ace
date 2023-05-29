#include "BoundNode/Statement/Jump/Normal.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement::Jump
{
    auto Normal::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Normal::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Normal>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Normal::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Normal>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Normal::Emit(Emitter& t_emitter) const -> void
    {
        t_emitter.GetBlockBuilder().Builder.CreateBr(
            t_emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol)
        );
    }
}
