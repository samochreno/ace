#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class RefExprSema :
        public std::enable_shared_from_this<RefExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<RefExprSema>,
        public virtual ILowerableSema<RefExprSema>
    {
    public:
        RefExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& expr
        );
        virtual ~RefExprSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const RefExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const RefExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprSema>;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSema> m_Expr{};
    };
}
