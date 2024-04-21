#pragma once

#include <memory>
#include <string>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class LiteralExprSema :
        public std::enable_shared_from_this<LiteralExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<LiteralExprSema>,
        public virtual ILowerableSema<LiteralExprSema>
    {
    public:
        LiteralExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const LiteralKind kind,
            const std::string& string
        );
        virtual ~LiteralExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const LiteralExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const LiteralExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
