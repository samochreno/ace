#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace::BoundNode::Expr::VarReference
{
    class Instance :
        public std::enable_shared_from_this<BoundNode::Expr::VarReference::Instance>,
        public virtual BoundNode::Expr::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expr::VarReference::Instance>,
        public virtual BoundNode::ILowerable<BoundNode::Expr::VarReference::Instance>
    {
    public:
        Instance(
            const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
            InstanceVarSymbol* const t_variableSymbol
        );
        virtual ~Instance() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Instance>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Instance>> final;
        auto GetOrCreateLoweredExpr(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const BoundNode::Expr::IBase>;
        auto GetVarSymbol() const -> InstanceVarSymbol*;

    private:
        std::shared_ptr<const BoundNode::Expr::IBase> m_Expr{};
        InstanceVarSymbol* m_VarSymbol{};
    };
}
