#pragma once

#include <memory>
#include <vector>

#include "Nodes/Node.hpp"
#include "Nodes/Exprs/StructConstructionExprNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class AttributeNode :
        public virtual INode,
        public virtual ICloneableNode<AttributeNode>,
        public virtual IBindableNode<AttributeBoundNode>
    {
    public:
        AttributeNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const StructConstructionExprNode>& structConstructionExpr
        );
        virtual ~AttributeNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const INode*> final;
        auto CloneInScope(
            const std::shared_ptr<Scope>& scope
        ) const -> std::shared_ptr<const AttributeNode> final;
        auto CreateBound() const -> Expected<std::shared_ptr<const AttributeBoundNode>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const StructConstructionExprNode> m_StructConstructionExpr{};
    };
}
