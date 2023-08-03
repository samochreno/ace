#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Scope.hpp"

namespace Ace
{
    class StructTypeBoundNode : 
        public std::enable_shared_from_this<StructTypeBoundNode>,
        public virtual ITypeBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<StructTypeBoundNode>,
        public virtual ITypeCheckableBoundNode<StructTypeBoundNode>,
        public virtual ILowerableBoundNode<StructTypeBoundNode>
    {
    public:
        StructTypeBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            StructTypeSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
            const std::vector<std::shared_ptr<const InstanceVarBoundNode>>& vars
        );
        virtual ~StructTypeBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const StructTypeBoundNode> final;
        auto CloneWithDiagnosticsType(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const ITypeBoundNode> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const StructTypeBoundNode>>> final;
        auto GetOrCreateTypeCheckedType(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const ITypeBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const StructTypeBoundNode>> final;
        auto GetOrCreateLoweredType(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const ITypeBoundNode>> final;

        auto GetSymbol() const -> StructTypeSymbol* final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        StructTypeSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
        std::vector<std::shared_ptr<const InstanceVarBoundNode>> m_Vars{};
    };
}
