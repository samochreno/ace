#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class DereferenceExprBoundNode :
        public std::enable_shared_from_this<DereferenceExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<DereferenceExprBoundNode>,
        public virtual ILowerableBoundNode<DereferenceExprBoundNode>
    {
    public:
        DereferenceExprBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_expr
        );
        virtual ~DereferenceExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const DereferenceExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const DereferenceExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprBoundNode>;

    private:
        std::shared_ptr<const IExprBoundNode> m_Expr{};
    };
}
