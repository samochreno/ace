#include "BoundNode/Variable/Normal/Static.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Attribute.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "NativeSymbol.hpp"

namespace Ace::BoundNode::Variable::Normal
{
    auto Static::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto Static::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Normal::Static>>>
    {
        ACE_TRY_ASSERT(m_Symbol->GetType() != NativeSymbol::Void.GetSymbol());

        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes, []
        (const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        if (!mchCheckedAttributes.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Variable::Normal::Static>(
            m_Symbol,
            mchCheckedAttributes.Value
        );

        return CreateChanged(returnValue);
    }

    auto Static::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Variable::Normal::Static>>>
    {
        ACE_TRY(mchLoweredAttributes, TransformExpectedMaybeChangedVector(m_Attributes, []
        (const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        }));

        if (!mchLoweredAttributes.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Variable::Normal::Static>(
            m_Symbol,
            mchLoweredAttributes.Value
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }
}
