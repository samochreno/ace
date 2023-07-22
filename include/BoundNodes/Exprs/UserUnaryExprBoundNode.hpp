#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "TypeInfo.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Assert.hpp"
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
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprBoundNode>& expr,
            FunctionSymbol* const opSymbol
        );
        virtual ~UserUnaryExprBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const UserUnaryExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprBoundNode> m_Expr{};
        FunctionSymbol* m_OpSymbol{};
    };
}
