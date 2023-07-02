#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
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
            const std::shared_ptr<Scope>& t_scope,
            const std::vector<std::shared_ptr<const FunctionBoundNode>>& t_functions
        );
        virtual ~ImplBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ImplBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ImplBoundNode>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const FunctionBoundNode>> m_Functions{};
    };
}
