#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Vars/VarSymbol.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class StaticVarRefExprSema :
        public std::enable_shared_from_this<StaticVarRefExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<StaticVarRefExprSema>,
        public virtual ILowerableSema<StaticVarRefExprSema>
    {
    public:
        StaticVarRefExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            IVarSymbol* const varSymbol
        );
        virtual ~StaticVarRefExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const StaticVarRefExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const StaticVarRefExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        IVarSymbol* m_VarSymbol{};
    };
}
