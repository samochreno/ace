#include "BoundNode/Stmt/Label.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Stmt
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
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Label>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Label::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Label::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Label>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Label::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
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
