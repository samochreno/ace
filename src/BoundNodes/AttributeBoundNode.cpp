#include "BoundNodes/AttributeBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    AttributeBoundNode::AttributeBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const StructConstructionExprBoundNode>& structConstructionExpr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_StructConstructionExpr{ structConstructionExpr }
    {
    }

    auto AttributeBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto AttributeBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AttributeBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto AttributeBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const AttributeBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const AttributeBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_StructConstructionExpr
        );
    }

    auto AttributeBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_StructConstructionExpr);

        return children;
    }

    auto AttributeBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const AttributeBoundNode>>>
    {
        ACE_TRY(cchCheckedStructureConstructionExpr, m_StructConstructionExpr->GetOrCreateTypeChecked({}));

        if (!cchCheckedStructureConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AttributeBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchCheckedStructureConstructionExpr.Value
        ));
    }

    auto AttributeBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const AttributeBoundNode>>
    {
        const auto cchLowewredStructConstructionExpr =
            m_StructConstructionExpr->GetOrCreateLowered({});

        if (!cchLowewredStructConstructionExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AttributeBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLowewredStructConstructionExpr.Value
        )->GetOrCreateLowered({}).Value);
    }
}
