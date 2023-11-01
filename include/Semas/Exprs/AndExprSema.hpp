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
    class AndExprSema :
        public std::enable_shared_from_this<AndExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<AndExprSema>,
        public virtual ILowerableSema<AndExprSema>
    {
    public:
        AndExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& lhsExpr,
            const std::shared_ptr<const IExprSema>& rhsExpr
        );
        virtual ~AndExprSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const AndExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const AndExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;
    
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSema> m_LHSExpr{};
        std::shared_ptr<const IExprSema> m_RHSExpr{};
    };
}
