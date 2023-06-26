#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"
#include "ExprEmitResult.hpp"

namespace Ace::BoundNode::Expr
{
    class UserUnary :
        public std::enable_shared_from_this<BoundNode::Expr::UserUnary>,
        public virtual BoundNode::Expr::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expr::UserUnary>,
        public virtual BoundNode::ILowerable<BoundNode::Expr::FunctionCall::Static>
    {
    public:
        UserUnary(
            const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
            FunctionSymbol* const t_operatorSymbol
        );
        virtual ~UserUnary() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::UserUnary>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Static>> final;
        auto GetOrCreateLoweredExpr(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<const BoundNode::Expr::IBase> m_Expr{};
        FunctionSymbol* m_OperatorSymbol{};
    };
}
