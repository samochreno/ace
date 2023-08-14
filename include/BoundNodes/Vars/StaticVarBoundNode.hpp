#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class StaticVarBoundNode :
        public std::enable_shared_from_this<StaticVarBoundNode>,
        public virtual IBoundNode,
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
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const StaticVarBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const StaticVarBoundNode> final;

        auto GetSymbol() const -> StaticVarSymbol*;

    private:
        SrcLocation m_SrcLocation{};
        StaticVarSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const AttributeBoundNode>> m_Attributes{};
    };
}
