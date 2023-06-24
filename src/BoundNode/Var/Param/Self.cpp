#include "BoundNode/Var/Param/Self.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Var::Param
{
    Self::Self(
        Symbol::Var::Param::Self* const t_symbol
    ) : m_Symbol{ t_symbol }
    {
    }

    auto Self::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Self::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Self::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Var::Param::Self>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        return CreateUnchanged(shared_from_this());
    }

    auto Self::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Var::Param::Self>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Self::GetSymbol() const -> Symbol::Var::Param::Self*
    {
        return m_Symbol;
    }
}
