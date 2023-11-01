#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class ConversionPlaceholderExprSema :
        public std::enable_shared_from_this<ConversionPlaceholderExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<ConversionPlaceholderExprSema>,
        public virtual ILowerableSema<ConversionPlaceholderExprSema>
    {
    public:
        ConversionPlaceholderExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const TypeInfo& typeInfo
        );
        virtual ~ConversionPlaceholderExprSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ConversionPlaceholderExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ConversionPlaceholderExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        TypeInfo m_TypeInfo{};
    };
}
