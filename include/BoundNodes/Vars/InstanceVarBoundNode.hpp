#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

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
            InstanceVarSymbol* const t_symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes
        );
        virtual ~InstanceVarBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const InstanceVarBoundNode>> final;

        auto GetSymbol() const -> InstanceVarSymbol* final;

    private:
        InstanceVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}
