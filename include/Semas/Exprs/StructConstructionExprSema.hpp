#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/FieldVarSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    struct StructConstructionExprSemaArg
    {
        auto operator<=>(const StructConstructionExprSemaArg&) const = default;

        FieldVarSymbol* Symbol{};
        std::shared_ptr<const IExprSema> Value{};
    };

    class StructConstructionExprSema :
        public std::enable_shared_from_this<StructConstructionExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<StructConstructionExprSema>,
        public virtual ILowerableSema<StructConstructionExprSema>
    {
    public:
        StructConstructionExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            StructTypeSymbol* const structSymbol,
            const std::vector<StructConstructionExprSemaArg>& args
        );
        virtual ~StructConstructionExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const StructConstructionExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const StructConstructionExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        StructTypeSymbol* m_StructSymbol{};
        std::vector<StructConstructionExprSemaArg> m_Args{};
    };
}
