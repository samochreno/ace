#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class CastExprSema : public std::enable_shared_from_this<CastExprSema>,
                         public virtual IExprSema,
                         public virtual ITypeCheckableSema<CastExprSema>,
                         public virtual ILowerableSema<CastExprSema>
    {
    public:
        CastExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& expr,
            ITypeSymbol* const typeSymbol
        );
        virtual ~CastExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(const TypeCheckingContext& context) const
            -> Diagnosed<std::shared_ptr<const CastExprSema>> final;
        auto CreateTypeCheckedExpr(const TypeCheckingContext& context) const
            -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(const LoweringContext& context) const
            -> std::shared_ptr<const CastExprSema> final;
        auto CreateLoweredExpr(const LoweringContext& context) const
            -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSema> m_Expr{};
        ITypeSymbol* m_TypeSymbol{};
    };
}
