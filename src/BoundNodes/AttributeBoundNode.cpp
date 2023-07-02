#include "BoundNodes/AttributeBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    AttributeBoundNode::AttributeBoundNode(
        const std::shared_ptr<const StructConstructionExprBoundNode>& t_structConstructionExpr
    ) : m_StructConstructionExpr{ t_structConstructionExpr }
    {
    }

    auto AttributeBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto AttributeBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_StructConstructionExpr);

        return children;
    }

    auto AttributeBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const AttributeBoundNode>>>
    {
        ACE_TRY(mchCheckedStructureConstructionExpr, m_StructConstructionExpr->GetOrCreateTypeChecked({}));

        if (!mchCheckedStructureConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const AttributeBoundNode>(
            mchCheckedStructureConstructionExpr.Value
        );
        return CreateChanged(returnValue);
    }

    auto AttributeBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const AttributeBoundNode>>
    {
        const auto mchLowewredStructConstructionExpr =
            m_StructConstructionExpr->GetOrCreateLowered({});

        if (!mchLowewredStructConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const AttributeBoundNode>(
            mchLowewredStructConstructionExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }
}
