#include "Nodes/AttributeNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Attribute.hpp"

namespace Ace
{
    AttributeNode::AttributeNode(
        const std::shared_ptr<const StructConstructionExprNode>& t_structConstructionExpr
    ) : m_StructConstructionExpr{ t_structConstructionExpr }
    {
    }

    auto AttributeNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_StructConstructionExpr->GetScope();
    }

    auto AttributeNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_StructConstructionExpr);

        return children;
    }

    auto AttributeNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const AttributeNode>
    {
        return std::make_shared<const AttributeNode>(
            m_StructConstructionExpr->CloneInScope(t_scope)
        );
    }

    auto AttributeNode::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Attribute>>
    {
        ACE_TRY(boundStructConstructionExpr, m_StructConstructionExpr->CreateBound());

        return std::make_shared<const BoundNode::Attribute>(
            boundStructConstructionExpr
        );
    }
}
