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
    class StaticFunctionCallExprBoundNode :
        public std::enable_shared_from_this<StaticFunctionCallExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<StaticFunctionCallExprBoundNode>,
        public virtual ILowerableBoundNode<StaticFunctionCallExprBoundNode>
    {
    public:
        StaticFunctionCallExprBoundNode(
            const std::shared_ptr<Scope>& t_scope,
            FunctionSymbol* const t_functionSymbol,
            const std::vector<std::shared_ptr<const IExprBoundNode>>& t_args
        );
        virtual ~StaticFunctionCallExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>> final;
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
        std::shared_ptr<Scope> m_Scope{};
        FunctionSymbol* m_FunctionSymbol{};
        std::vector<std::shared_ptr<const IExprBoundNode>> m_Args{};
    };
}
