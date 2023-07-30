#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/ModuleSymbol.hpp"
#include "Scope.hpp"
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
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            ModuleSymbol* const symbol,
            const std::vector<std::shared_ptr<const ModuleBoundNode>>& modules,
            const std::vector<std::shared_ptr<const ITypeBoundNode>>& types,
            const std::vector<std::shared_ptr<const ImplBoundNode>>& impls,
            const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions,
            const std::vector<std::shared_ptr<const StaticVarBoundNode>>& vars
        );
        virtual ~ModuleBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ModuleBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const ModuleBoundNode>> final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        ModuleSymbol* m_Symbol{};
        std::vector<std::shared_ptr<const ModuleBoundNode>> m_Modules{};
        std::vector<std::shared_ptr<const ITypeBoundNode>> m_Types{};
        std::vector<std::shared_ptr<const ImplBoundNode>> m_Impls{};
        std::vector<std::shared_ptr<const FunctionBoundNode>> m_Functions{};
        std::vector<std::shared_ptr<const StaticVarBoundNode>> m_Vars{};
    };
}
