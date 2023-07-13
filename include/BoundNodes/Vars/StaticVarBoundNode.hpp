#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class StaticVarBoundNode :
        public std::enable_shared_from_this<StaticVarBoundNode>,
        public virtual IBoundNode,
        public virtual ITypedBoundNode<StaticVarSymbol>,
        public virtual ITypeCheckableBoundNode<StaticVarBoundNode>,
        public virtual ILowerableBoundNode<StaticVarBoundNode>
    {
    public:
        StaticVarBoundNode(
            StaticVarSymbol* const t_symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes
        );
        virtual ~StaticVarBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const StaticVarBoundNode>> final;

        auto GetSymbol() const -> StaticVarSymbol* final;

    private:
        StaticVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}
