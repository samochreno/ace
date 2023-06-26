#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace::BoundNode::Expr
{
    class DerefAs :
        public std::enable_shared_from_this<BoundNode::Expr::DerefAs>,
        public virtual BoundNode::Expr::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expr::DerefAs>,
        public virtual BoundNode::ILowerable<BoundNode::Expr::DerefAs>
    {
    public:
        DerefAs(
            const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
            ITypeSymbol* const t_typeSymbol
        );
        virtual ~DerefAs() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::DerefAs>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::DerefAs>> final;
        auto GetOrCreateLoweredExpr(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<const BoundNode::Expr::IBase> m_Expr{};
        ITypeSymbol* m_TypeSymbol{};
    };
}
