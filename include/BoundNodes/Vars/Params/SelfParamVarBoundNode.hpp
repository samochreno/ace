#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

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
        SelfParamVarBoundNode(
            const SrcLocation& srcLocation,
            SelfParamVarSymbol* const symbol
        );
        virtual ~SelfParamVarBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const SelfParamVarBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const SelfParamVarBoundNode> final;

        auto GetSymbol() const -> SelfParamVarSymbol* final;

    private:
        SrcLocation m_SrcLocation{};
        SelfParamVarSymbol* m_Symbol{};
    };
}
