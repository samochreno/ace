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
    class SizeOf :
        public std::enable_shared_from_this<BoundNode::Expr::SizeOf>,
        public virtual BoundNode::Expr::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expr::SizeOf>,
        public virtual BoundNode::ILowerable<BoundNode::Expr::SizeOf>
    {
    public:
        SizeOf(
            const std::shared_ptr<Scope>& t_scope,
            ITypeSymbol* const t_typeSymbol
        );
        virtual ~SizeOf() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::SizeOf>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::SizeOf>> final;
        auto GetOrCreateLoweredExpr(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        ITypeSymbol* m_TypeSymbol{};
    };
}
