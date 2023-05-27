#include "BoundNode/Impl.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Function.hpp"
#include "BoundNode/Variable/Normal/Static.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    auto Impl::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Functions);

        return children;
    }

    auto Impl::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Impl>>>
    {
        ACE_TRY(mchCheckedFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const BoundNode::Function>& t_function)
        {
            return t_function->GetOrCreateTypeChecked({});
        }));

        if (!mchCheckedFunctions.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Impl>(
            m_Scope,
            mchCheckedFunctions.Value
            );

        return CreateChanged(returnValue);
    }

    auto Impl::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Impl>>>
    {
        ACE_TRY(mchLoweredFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const BoundNode::Function>& t_function)
        {
            return t_function->GetOrCreateLowered({});
        }));

        if (!mchLoweredFunctions.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Impl>(
            m_Scope,
            mchLoweredFunctions.Value
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }
}
