#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Cacheable.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class DerefExprBoundNode :
        public std::enable_shared_from_this<DerefExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ICloneableWithDiagnosticsBoundNode<DerefExprBoundNode>,
        public virtual ITypeCheckableBoundNode<DerefExprBoundNode>,
        public virtual ILowerableBoundNode<DerefExprBoundNode>
    {
    public:
        DerefExprBoundNode(
            const DiagnosticBag& diagnostics,
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& expr
        );
        virtual ~DerefExprBoundNode() = default;

        auto GetDiagnostics() const -> const DiagnosticBag& final;
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CloneWithDiagnostics(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const DerefExprBoundNode> final;
        auto CloneWithDiagnosticsExpr(
            DiagnosticBag diagnostics
        ) const -> std::shared_ptr<const IExprBoundNode> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const DerefExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const DerefExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprBoundNode>;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_Expr{};
    };
}
