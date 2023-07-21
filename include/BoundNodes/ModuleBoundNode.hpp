#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Symbols/ModuleSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

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
            const SourceLocation& t_sourceLocation,
            ModuleSymbol* const t_symbol,
            const std::vector<std::shared_ptr<const ModuleBoundNode>>& t_modules,
            const std::vector<std::shared_ptr<const ITypeBoundNode>>& t_types,
            const std::vector<std::shared_ptr<const ImplBoundNode>>& t_impls,
            const std::vector<std::shared_ptr<const FunctionBoundNode>>& t_functions,
            const std::vector<std::shared_ptr<const StaticVarBoundNode>>& t_vars
        );
        virtual ~ModuleBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ModuleBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ModuleBoundNode>> final;

    private:
        SourceLocation m_SourceLocation{};
        ModuleSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const ModuleBoundNode>> m_Modules{};
        std::vector<std::shared_ptr<const ITypeBoundNode>> m_Types{};
        std::vector<std::shared_ptr<const ImplBoundNode>> m_Impls{};
        std::vector<std::shared_ptr<const FunctionBoundNode>> m_Functions{};
        std::vector<std::shared_ptr<const StaticVarBoundNode>> m_Vars{};
    };
}
