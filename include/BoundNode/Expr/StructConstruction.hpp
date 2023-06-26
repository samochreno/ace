#pragma once

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "Scope.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace::BoundNode::Expr
{
    class StructConstruction :
        public std::enable_shared_from_this<BoundNode::Expr::StructConstruction>,
        public virtual BoundNode::Expr::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expr::StructConstruction>,
        public virtual BoundNode::ILowerable<BoundNode::Expr::StructConstruction>
    {
    public:
        struct Arg
        {
            InstanceVarSymbol* Symbol{};
            std::shared_ptr<const BoundNode::Expr::IBase> Value{};
        };

        StructConstruction(
            const std::shared_ptr<Scope>& t_scope,
            StructTypeSymbol* const t_structSymbol,
            const std::vector<Arg>& t_args
        );
        virtual ~StructConstruction() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::StructConstruction>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const BoundNode::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>> final;
        auto GetOrCreateLowered(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::StructConstruction>> final;
        auto GetOrCreateLoweredExpr(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        StructTypeSymbol* m_StructSymbol{};
        std::vector<Arg> m_Args{};
    };
}
