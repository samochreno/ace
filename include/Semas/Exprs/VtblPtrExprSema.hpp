#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class VtblPtrExprSema :
        public std::enable_shared_from_this<VtblPtrExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<VtblPtrExprSema>,
        public virtual ILowerableSema<VtblPtrExprSema>
    {
    public:
        VtblPtrExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            ISizedTypeSymbol* const typeSymbol,
            ITypeSymbol* const traitSymbol
        );
        virtual ~VtblPtrExprSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const VtblPtrExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const VtblPtrExprSema> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        ISizedTypeSymbol* m_TypeSymbol{};
        ITypeSymbol* m_TraitSymbol{};
    };
}
