#include "BoundNode/Statement/BlockEnd.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Symbol/Variable/Local.hpp"
#include "ExpressionDropData.hpp"

namespace  Ace::BoundNode::Statement
{
    auto BlockEnd::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto BlockEnd::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::BlockEnd>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEnd::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::BlockEnd>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEnd::Emit(Emitter& t_emitter) const -> void
    {
    }
}
