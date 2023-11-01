#pragma once

#include <memory>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class TypeInfoPtrExprSema :
        public std::enable_shared_from_this<TypeInfoPtrExprSema>,
        public virtual IExprSema,
        public virtual ITypeCheckableSema<TypeInfoPtrExprSema>,
        public virtual ILowerableSema<TypeInfoPtrExprSema>
    {
    public:
        TypeInfoPtrExprSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            ISizedTypeSymbol* const typeSymbol
        );
        virtual ~TypeInfoPtrExprSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const TypeInfoPtrExprSema>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const TypeInfoPtrExprSema> final;
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
    };
}
