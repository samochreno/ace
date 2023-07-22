#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/TypedBoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "SrcLocation.hpp"
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
            const SrcLocation& srcLocation,
            StaticVarSymbol* const symbol,
            const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
        );
        virtual ~StaticVarBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticVarBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const StaticVarBoundNode>> final;

        auto GetSymbol() const -> StaticVarSymbol* final;

    private:
        SrcLocation m_SrcLocation{};
        StaticVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}
