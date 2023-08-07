#include "Nodes/AttributeNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"

namespace Ace
{
    AttributeNode::AttributeNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const StructConstructionExprNode>& structConstructionExpr
    ) : m_SrcLocation{ srcLocation },
        m_StructConstructionExpr{ structConstructionExpr }
    {
    }

    auto AttributeNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AttributeNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto AttributeNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_StructConstructionExpr);

        return children;
    }

    auto AttributeNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const AttributeNode>
    {
        return std::make_shared<const AttributeNode>(
            m_SrcLocation,
            m_StructConstructionExpr->CloneInScope(scope)
        );
    }

    auto AttributeNode::CreateBound() const -> Diagnosed<std::shared_ptr<const AttributeBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto boundStructConstructionExpr =
            diagnostics.Collect(m_StructConstructionExpr->CreateBound());

        return Diagnosed
        {
            std::make_shared<const AttributeBoundNode>(
                GetSrcLocation(),
                boundStructConstructionExpr
            ),
            diagnostics,
        };
    }
}
