#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class SelfParamVarBoundNode : 
        public std::enable_shared_from_this<SelfParamVarBoundNode>,
        public virtual IBoundNode,
        public virtual ITypedBoundNode<SelfParamVarSymbol>,
        public virtual ITypeCheckableBoundNode<SelfParamVarBoundNode>,
        public virtual ILowerableBoundNode<SelfParamVarBoundNode>
    {
    public:
        SelfParamVarBoundNode(SelfParamVarSymbol* const t_symbol);
        virtual ~SelfParamVarBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const SelfParamVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const SelfParamVarBoundNode>> final;

        auto GetSymbol() const -> SelfParamVarSymbol* final;

    private:
        SelfParamVarSymbol* m_Symbol{};
    };
}
