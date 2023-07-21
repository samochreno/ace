#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Scope.hpp"

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
            const SourceLocation& sourceLocation,
            StructTypeSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
            const std::vector<std::shared_ptr<const InstanceVarBoundNode>>& vars
        );
        virtual ~StructTypeBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StructTypeBoundNode>>> final;
        auto GetOrCreateTypeCheckedType(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ITypeBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const StructTypeBoundNode>> final;
        auto GetOrCreateLoweredType(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ITypeBoundNode>> final;

        auto GetSymbol() const -> StructTypeSymbol* final;

    private:
        SourceLocation m_SourceLocation{};
        StructTypeSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
        std::vector<std::shared_ptr<const InstanceVarBoundNode>> m_Vars{};
    };
}
