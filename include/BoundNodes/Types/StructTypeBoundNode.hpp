#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"

namespace Ace
{
    class StructTypeBoundNode : 
        public std::enable_shared_from_this<StructTypeBoundNode>,
        public virtual ITypeBoundNode,
        public virtual ITypeCheckableBoundNode<StructTypeBoundNode>,
        public virtual ILowerableBoundNode<StructTypeBoundNode>
    {
    public:
        StructTypeBoundNode(
            const SrcLocation& srcLocation,
            StructTypeSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
            const std::vector<std::shared_ptr<const InstanceVarBoundNode>>& vars
        );
        virtual ~StructTypeBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const StructTypeBoundNode>> final;
        auto CreateTypeCheckedType(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ITypeBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const StructTypeBoundNode> final;
        auto CreateLoweredType(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ITypeBoundNode> final;

        auto GetSymbol() const -> StructTypeSymbol* final;

    private:
        SrcLocation m_SrcLocation{};
        StructTypeSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
        std::vector<std::shared_ptr<const InstanceVarBoundNode>> m_Vars{};
    };
}
