#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "Scope.hpp"
#include "Symbol/Function.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Asserts.hpp"
#include "ExprEmitResult.hpp"

namespace Ace::BoundNode::Expr
{
    class UserBinary :
        public std::enable_shared_from_this<BoundNode::Expr::UserBinary>,
        public virtual BoundNode::Expr::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expr::UserBinary>,
        public virtual BoundNode::ILowerable<BoundNode::Expr::FunctionCall::Static>
    {
    public:
        UserBinary(
            const std::shared_ptr<const BoundNode::Expr::IBase>& t_lhsExpr,
            const std::shared_ptr<const BoundNode::Expr::IBase>& t_rhsExpr,
            Symbol::Function* const t_operatorSymbol
        );
        virtual ~UserBinary() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::UserBinary>>> final;
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
        std::shared_ptr<const BoundNode::Expr::IBase> m_LHSExpr{};
        std::shared_ptr<const BoundNode::Expr::IBase> m_RHSExpr{};
        Symbol::Function* m_OperatorSymbol{};
    };
}
