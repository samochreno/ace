#include "BoundNode/Stmt/Jump/Normal.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Stmt::Jump
{
    Normal::Normal(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Label* const t_labelSymbol
    ) : m_Scope{ t_scope },
        m_LabelSymbol{ t_labelSymbol }
    {
    }

    auto Normal::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Normal::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Normal::GetOrCreateTypeChecked(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Jump::Normal>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Normal::GetOrCreateTypeCheckedStmt(
        const BoundNode::Stmt::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Normal::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::Jump::Normal>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Normal::GetOrCreateLoweredStmt(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Normal::Emit(Emitter& t_emitter) const -> void
    {
        t_emitter.GetBlockBuilder().Builder.CreateBr(
            t_emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol)
        );
    }

    auto Normal::GetLabelSymbol() const -> Symbol::Label*
    {
        return m_LabelSymbol;
    }
}
