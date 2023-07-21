#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class AttributeBoundNode : 
        public std::enable_shared_from_this<AttributeBoundNode>,
        public virtual IBoundNode, 
        public virtual ITypeCheckableBoundNode<AttributeBoundNode>, 
        public virtual ILowerableBoundNode<AttributeBoundNode>
    {
    public:
        AttributeBoundNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<const StructConstructionExprBoundNode>& structConstructionExpr
        );
        virtual ~AttributeBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const AttributeBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const AttributeBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<const StructConstructionExprBoundNode> m_StructConstructionExpr{};
    };
}
