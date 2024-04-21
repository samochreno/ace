#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/FieldVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class FieldVarRefExprSema :
        public std::enable_shared_from_this<FieldVarRefExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<FieldVarRefExprSema>,
        public virtual ILowerableSema<FieldVarRefExprSema>
    {
    public:
        FieldVarRefExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& expr,
            FieldVarSymbol* const fieldSymbol 
        );
        virtual ~FieldVarRefExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const FieldVarRefExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const FieldVarRefExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetExpr() const -> std::shared_ptr<const IExprSema>;
        auto GetFieldSymbol() const -> FieldVarSymbol*;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSema> m_Expr{};
        FieldVarSymbol* m_FieldSymbol{};
    };
}
