#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class InstanceFunctionCallExprBoundNode :
        public std::enable_shared_from_this<InstanceFunctionCallExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<InstanceFunctionCallExprBoundNode>,
        public virtual ILowerableBoundNode<InstanceFunctionCallExprBoundNode>
    {
    public:
        InstanceFunctionCallExprBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_expr,
            FunctionSymbol* const t_functionSymbol,
            const std::vector<std::shared_ptr<const IExprBoundNode>>& t_args
        );
        virtual ~InstanceFunctionCallExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const InstanceFunctionCallExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<const IExprBoundNode> m_Expr{};
        FunctionSymbol* m_FunctionSymbol{};
        std::vector<std::shared_ptr<const IExprBoundNode>> m_Args{};
    };
}
