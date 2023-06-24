#include "BoundNode/Statement/BlockEnd.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Symbol/Variable/Local.hpp"
#include "ExpressionDropData.hpp"

namespace Ace::BoundNode::Statement
{
    BlockEnd::BlockEnd(
        const std::shared_ptr<Scope>& t_selfScope
    ) : m_SelfScope{ t_selfScope }
    {
    }

    auto BlockEnd::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto BlockEnd::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope;
    }

    auto BlockEnd::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto BlockEnd::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::BlockEnd>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEnd::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto BlockEnd::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::BlockEnd>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto BlockEnd::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto BlockEnd::Emit(Emitter& t_emitter) const -> void
    {
    }
}
