#include "BoundNodes/AttributeBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    AttributeBoundNode::AttributeBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const StructConstructionExprBoundNode>& structConstructionExpr
    ) : m_SrcLocation{ srcLocation },
        m_StructConstructionExpr{ structConstructionExpr }
    {
    }

    auto AttributeBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AttributeBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto AttributeBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_StructConstructionExpr);

        return children;
    }

    auto AttributeBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const AttributeBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedStructConstructionExpr =
            diagnostics.Collect(m_StructConstructionExpr->CreateTypeChecked({}));

        if (checkedStructConstructionExpr == m_StructConstructionExpr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const AttributeBoundNode>(
                GetSrcLocation(),
                checkedStructConstructionExpr
            ),
            std::move(diagnostics),
        };
    }

    auto AttributeBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const AttributeBoundNode>
    {
        const auto loweredStructConstructionExpr =
            m_StructConstructionExpr->CreateLowered({});

        if (loweredStructConstructionExpr == m_StructConstructionExpr)
        {
            return shared_from_this();
        }

        return std::make_shared<const AttributeBoundNode>(
            GetSrcLocation(),
            loweredStructConstructionExpr
        )->CreateLowered({});
    }
}
