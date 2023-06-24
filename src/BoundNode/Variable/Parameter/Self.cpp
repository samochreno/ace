#include "BoundNode/Variable/Parameter/Self.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Variable::Parameter
{
    Self::Self(
        Symbol::Variable::Parameter::Self* const t_symbol
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
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Self>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        return CreateUnchanged(shared_from_this());
    }

    auto Self::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Self>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Self::GetSymbol() const -> Symbol::Variable::Parameter::Self*
    {
        return m_Symbol;
    }
}
