#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class UserUnaryExprBoundNode :
        public std::enable_shared_from_this<UserUnaryExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<UserUnaryExprBoundNode>,
        public virtual ILowerableBoundNode<StaticFunctionCallExprBoundNode>
    {
    public:
        UserUnaryExprBoundNode(
            const std::shared_ptr<const IExprBoundNode>& t_expr,
            FunctionSymbol* const t_operatorSymbol
        );
        virtual ~UserUnaryExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const UserUnaryExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<const IExprBoundNode> m_Expr{};
        FunctionSymbol* m_OperatorSymbol{};
    };
}