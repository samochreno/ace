#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class ImplBoundNode : 
        public std::enable_shared_from_this<ImplBoundNode>,
        public virtual IBoundNode,
        public virtual ITypeCheckableBoundNode<ImplBoundNode>,
        public virtual ILowerableBoundNode<ImplBoundNode>
    {
    public:
        ImplBoundNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions
        );
        virtual ~ImplBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ImplBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ImplBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const FunctionBoundNode>> m_Functions{};
    };
}
