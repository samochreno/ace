#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class UserUnaryExprSema :
        public std::enable_shared_from_this<UserUnaryExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<UserUnaryExprSema>,
        public virtual ILowerableSema<StaticCallExprSema>
    {
    public:
        UserUnaryExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& expr,
            FunctionSymbol* const opSymbol
        );
        virtual ~UserUnaryExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const UserUnaryExprSema>> final;
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
        std::shared_ptr<const IExprSema> m_Expr{};
        FunctionSymbol* m_OpSymbol{};
    };
}
