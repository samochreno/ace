#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class InstanceVarBoundNode : 
        public std::enable_shared_from_this<InstanceVarBoundNode>,
        public virtual IBoundNode,
        public virtual ITypedBoundNode<InstanceVarSymbol>,
        public virtual ITypeCheckableBoundNode<InstanceVarBoundNode>,
        public virtual ILowerableBoundNode<InstanceVarBoundNode>
    {
    public:
        InstanceVarBoundNode(
            const SrcLocation& srcLocation,
            InstanceVarSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
        );
        virtual ~InstanceVarBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const InstanceVarBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const InstanceVarBoundNode> final;

        auto GetSymbol() const -> InstanceVarSymbol* final;

    private:
        SrcLocation m_SrcLocation{};
        InstanceVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}
