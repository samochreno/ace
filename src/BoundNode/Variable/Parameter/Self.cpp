#include "BoundNode/Variable/Parameter/Self.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Variable::Parameter
{
    auto Self::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Self::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Self>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        return CreateUnchanged(shared_from_this());
    }

    auto Self::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Variable::Parameter::Self>>
    {
        return CreateUnchanged(shared_from_this());
    }
}
