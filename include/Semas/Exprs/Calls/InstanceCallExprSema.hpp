#pragma once

#include <memory>
#include <vector>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/CallableSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class InstanceCallExprSema :
        public std::enable_shared_from_this<InstanceCallExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<InstanceCallExprSema>,
        public virtual ILowerableSema<InstanceCallExprSema>
    {
    public:
        InstanceCallExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& expr,
            ICallableSymbol* const callableSymbol,
            const std::vector<std::shared_ptr<const IExprSema>>& args
        );
        virtual ~InstanceCallExprSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const InstanceCallExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const InstanceCallExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSema> m_Expr{};
        ICallableSymbol* m_CallableSymbol{};
        std::vector<std::shared_ptr<const IExprSema>> m_Args{};
    };
}
