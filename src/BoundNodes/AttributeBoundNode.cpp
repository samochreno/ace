#include "BoundNodes/AttributeBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    AttributeBoundNode::AttributeBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const StructConstructionExprBoundNode>& structConstructionExpr
    ) : m_SourceLocation{ sourceLocation },
        m_StructConstructionExpr{ structConstructionExpr }
    {
    }

    auto AttributeBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const AttributeBoundNode>>>
    {
        ACE_TRY(mchCheckedStructureConstructionExpr, m_StructConstructionExpr->GetOrCreateTypeChecked({}));

        if (!mchCheckedStructureConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AttributeBoundNode>(
            GetSourceLocation(),
            mchCheckedStructureConstructionExpr.Value
        ));
    }

    auto AttributeBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const AttributeBoundNode>>
    {
        const auto mchLowewredStructConstructionExpr =
            m_StructConstructionExpr->GetOrCreateLowered({});

        if (!mchLowewredStructConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AttributeBoundNode>(
            GetSourceLocation(),
            mchLowewredStructConstructionExpr.Value
        )->GetOrCreateLowered({}).Value);
    }
}
