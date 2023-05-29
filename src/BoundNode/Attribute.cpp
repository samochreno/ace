#include "BoundNode/Attribute.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    auto Attribute::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_StructConstructionExpression);

        return children;
    }

    auto Attribute::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Attribute>>>
    {
        ACE_TRY(mchCheckedStructureConstructionExpression, m_StructConstructionExpression->GetOrCreateTypeChecked({}));

        if (!mchCheckedStructureConstructionExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Attribute>(
            mchCheckedStructureConstructionExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto Attribute::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Attribute>>
    {
        const auto mchLowewredStructConstructionExpression = m_StructConstructionExpression->GetOrCreateLowered({});

        if (!mchLowewredStructConstructionExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Attribute>(
            mchLowewredStructConstructionExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }
}
