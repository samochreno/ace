#include "BoundNode/Statement/Label.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    auto Label::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Label::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Label>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Label::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Label>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Label::Emit(Emitter& t_emitter) const -> void
    {
    }
}
