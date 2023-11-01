#pragma once

#include <memory>

#include "Semas/Sema.hpp"
#include "Emittable.hpp"
#include "ExprEmitResult.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "TypeConversions.hpp"

namespace Ace
{
    class IExprSema :
        public virtual ISema,
        public virtual IEmittable<ExprEmitResult>
    {
    public:
        virtual ~IExprSema() = default;

        virtual auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprSema>> = 0;
        virtual auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprSema> = 0;

        virtual auto GetTypeInfo() const -> TypeInfo = 0;
    };
}
