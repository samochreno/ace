#include "BoundNode/Statement/Label.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    Label::Label(Symbol::Label* const t_symbol) 
        : m_Symbol{ t_symbol }
    {
    }

    auto Label::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Label::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Label::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Label>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Label::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Label::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Label>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Label::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Label::Emit(Emitter& t_emitter) const -> void
    {
    }

    auto Label::GetLabelSymbol() const -> Symbol::Label*
    {
        return m_Symbol;
    }
}
