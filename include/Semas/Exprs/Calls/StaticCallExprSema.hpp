#pragma once

#include <memory>
#include <vector>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/CallableSymbol.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class StaticCallExprSema :
        public std::enable_shared_from_this<StaticCallExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<StaticCallExprSema>,
        public virtual ILowerableSema<StaticCallExprSema>
    {
    public:
        StaticCallExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            ICallableSymbol* const callableSymbol,
            const std::vector<std::shared_ptr<const IExprSema>>& args
        );
        virtual ~StaticCallExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const StaticCallExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const StaticCallExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        ICallableSymbol* m_CallableSymbol{};
        std::vector<std::shared_ptr<const IExprSema>> m_Args{};
    };
}
