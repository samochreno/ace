#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/ModuleSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ModuleBoundNode : 
        public std::enable_shared_from_this<ModuleBoundNode>,
        public virtual IBoundNode,
        public virtual ITypeCheckableBoundNode<ModuleBoundNode>,
        public virtual ILowerableBoundNode<ModuleBoundNode>
    {
    public:
        ModuleBoundNode(
            const SrcLocation& srcLocation,
            ModuleSymbol* const symbol,
            const std::vector<std::shared_ptr<const ModuleBoundNode>>& modules,
            const std::vector<std::shared_ptr<const ITypeBoundNode>>& types,
            const std::vector<std::shared_ptr<const ImplBoundNode>>& impls,
            const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions,
            const std::vector<std::shared_ptr<const StaticVarBoundNode>>& vars
        );
        virtual ~ModuleBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ModuleBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ModuleBoundNode> final;

    private:
        SrcLocation m_SrcLocation{};
        ModuleSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const ModuleBoundNode>> m_Modules{};
        std::vector<std::shared_ptr<const ITypeBoundNode>> m_Types{};
        std::vector<std::shared_ptr<const ImplBoundNode>> m_Impls{};
        std::vector<std::shared_ptr<const FunctionBoundNode>> m_Functions{};
        std::vector<std::shared_ptr<const StaticVarBoundNode>> m_Vars{};
    };
}
