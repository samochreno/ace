#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/Exprs/StructConstructionExprNode.hpp"
#include "BoundNode/Attribute.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class AttributeNode :
        public virtual INode,
        public virtual ICloneableNode<AttributeNode>,
        public virtual IBindableNode<BoundNode::Attribute>
    {
    public:
        AttributeNode(
            const std::shared_ptr<const StructConstructionExprNode>& t_structConstructionExpr
        );
        virtual ~AttributeNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& t_scope
        ) const -> std::shared_ptr<const AttributeNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Attribute>> final;

    private:
        std::shared_ptr<const StructConstructionExprNode> m_StructConstructionExpr{};
    };
}
