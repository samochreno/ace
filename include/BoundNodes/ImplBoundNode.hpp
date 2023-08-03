#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    class ImplBoundNode : 
        public std::enable_shared_from_this<ImplBoundNode>,
        public virtual IBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<ImplBoundNode>,
        public virtual ITypeCheckableBoundNode<ImplBoundNode>,
        public virtual ILowerableBoundNode<ImplBoundNode>
    {
    public:
        ImplBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions
        );
        virtual ~ImplBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const ImplBoundNode> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const ImplBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const ImplBoundNode>> final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const FunctionBoundNode>> m_Functions{};
    };
}
