#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class ParamVarBoundNode : 
        public std::enable_shared_from_this<ParamVarBoundNode>,
        public virtual IBoundNode,
        public virtual ITypedBoundNode<NormalParamVarSymbol>,
        public virtual ITypeCheckableBoundNode<ParamVarBoundNode>,
        public virtual ILowerableBoundNode<ParamVarBoundNode>
    {
    public:
        ParamVarBoundNode(
            NormalParamVarSymbol* const t_symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes
        );
        virtual ~ParamVarBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ParamVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ParamVarBoundNode>> final;

        auto GetSymbol() const -> NormalParamVarSymbol* final;

    private:
        NormalParamVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}