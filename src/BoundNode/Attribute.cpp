#include "BoundNode/Attribute.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    Attribute::Attribute(
        const std::shared_ptr<const BoundNode::Expr::StructConstruction>& t_structConstructionExpr
    ) : m_StructConstructionExpr{ t_structConstructionExpr }
    {
    }

    auto Attribute::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto Attribute::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_StructConstructionExpr);

        return children;
    }

    auto Attribute::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Attribute>>>
    {
        ACE_TRY(mchCheckedStructureConstructionExpr, m_StructConstructionExpr->GetOrCreateTypeChecked({}));

        if (!mchCheckedStructureConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Attribute>(
            mchCheckedStructureConstructionExpr.Value
        );
        return CreateChanged(returnValue);
    }

    auto Attribute::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Attribute>>
    {
        const auto mchLowewredStructConstructionExpr =
            m_StructConstructionExpr->GetOrCreateLowered({});

        if (!mchLowewredStructConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Attribute>(
            mchLowewredStructConstructionExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }
}
