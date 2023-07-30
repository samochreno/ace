#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Scope.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class NormalParamVarBoundNode : 
        public std::enable_shared_from_this<NormalParamVarBoundNode>,
        public virtual IBoundNode,
        public virtual ITypedBoundNode<NormalParamVarSymbol>,
        public virtual ITypeCheckableBoundNode<NormalParamVarBoundNode>,
        public virtual ILowerableBoundNode<NormalParamVarBoundNode>
    {
    public:
        NormalParamVarBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            NormalParamVarSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
        );
        virtual ~NormalParamVarBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const NormalParamVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const NormalParamVarBoundNode>> final;

        auto GetSymbol() const -> NormalParamVarSymbol* final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        NormalParamVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}
