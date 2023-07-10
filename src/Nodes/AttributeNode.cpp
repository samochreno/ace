#include "Nodes/AttributeNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"

namespace Ace
{
    AttributeNode::AttributeNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const StructConstructionExprNode>& t_structConstructionExpr
    ) : m_SourceLocation{ t_sourceLocation },
        m_StructConstructionExpr{ t_structConstructionExpr }
    {
    }

    auto AttributeNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
            m_SourceLocation,
            m_StructConstructionExpr->CloneInScope(t_scope)
        );
    }

    auto AttributeNode::CreateBound() const -> Expected<std::shared_ptr<const AttributeBoundNode>>
    {
        ACE_TRY(boundStructConstructionExpr, m_StructConstructionExpr->CreateBound());

        return std::make_shared<const AttributeBoundNode>(
            boundStructConstructionExpr
        );
    }
}
