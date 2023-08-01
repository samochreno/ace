#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    struct StructConstructionExprBoundArg
    {
        InstanceVarSymbol* Symbol{};
        std::shared_ptr<const IExprBoundNode> Value{};
    };

    class StructConstructionExprBoundNode :
        public std::enable_shared_from_this<StructConstructionExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<StructConstructionExprBoundNode>,
        public virtual ITypeCheckableBoundNode<StructConstructionExprBoundNode>,
        public virtual ILowerableBoundNode<StructConstructionExprBoundNode>
    {
    public:
        StructConstructionExprBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            StructTypeSymbol* const structSymbol,
            const std::vector<StructConstructionExprBoundArg>& args
        );
        virtual ~StructConstructionExprBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const StructConstructionExprBoundNode> final;
        auto CloneWithDiagnosticsExpr(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const IExprBoundNode> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StructConstructionExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const StructConstructionExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        StructTypeSymbol* m_StructSymbol{};
        std::vector<StructConstructionExprBoundArg> m_Args{};
    };
}
