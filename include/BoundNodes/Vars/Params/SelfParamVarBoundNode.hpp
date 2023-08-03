#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

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
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            SelfParamVarSymbol* const symbol
        );
        virtual ~SelfParamVarBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const SelfParamVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const SelfParamVarBoundNode>> final;

        auto GetSymbol() const -> SelfParamVarSymbol* final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        SelfParamVarSymbol* m_Symbol{};
    };
}
