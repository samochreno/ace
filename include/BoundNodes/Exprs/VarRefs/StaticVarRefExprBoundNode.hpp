#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Vars/VarSymbol.hpp"
#include "Cacheable.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class StaticVarRefExprBoundNode :
        public std::enable_shared_from_this<StaticVarRefExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<StaticVarRefExprBoundNode>,
        public virtual ITypeCheckableBoundNode<StaticVarRefExprBoundNode>,
        public virtual ILowerableBoundNode<StaticVarRefExprBoundNode>
    {
    public:
        StaticVarRefExprBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            IVarSymbol* const varSymbol
        );
        virtual ~StaticVarRefExprBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const StaticVarRefExprBoundNode> final;
        auto CloneWithDiagnosticsExpr(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const IExprBoundNode> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const StaticVarRefExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const StaticVarRefExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetVarSymbol() const -> IVarSymbol*;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        IVarSymbol* m_VarSymbol{};
    };
}
