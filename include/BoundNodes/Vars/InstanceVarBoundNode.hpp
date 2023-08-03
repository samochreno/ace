#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

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
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            InstanceVarSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
        );
        virtual ~InstanceVarBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const InstanceVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const InstanceVarBoundNode>> final;

        auto GetSymbol() const -> InstanceVarSymbol* final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        InstanceVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}
