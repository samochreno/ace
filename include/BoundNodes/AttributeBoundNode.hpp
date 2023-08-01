#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class AttributeBoundNode : 
        public std::enable_shared_from_this<AttributeBoundNode>,
        public virtual IBoundNode, 
        public virtual ICloneableWithDiagnosticsBoundNode<AttributeBoundNode>,
        public virtual ITypeCheckableBoundNode<AttributeBoundNode>, 
        public virtual ILowerableBoundNode<AttributeBoundNode>
    {
    public:
        AttributeBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<const StructConstructionExprBoundNode>& structConstructionExpr
        );
        virtual ~AttributeBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const AttributeBoundNode> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const AttributeBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const AttributeBoundNode>> final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const StructConstructionExprBoundNode> m_StructConstructionExpr{};
    };
}
