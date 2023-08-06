#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "BoundNodes/BoundNode.hpp"
#include "Emittable.hpp"
#include "ExprEmitResult.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "TypeConversions.hpp"

namespace Ace
{
    class IExprBoundNode :
        public virtual IBoundNode,
        public virtual IEmittable<ExprEmitResult>
    {
    public:
        virtual ~IExprBoundNode() = default;

        virtual auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>> = 0;
        virtual auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprBoundNode> = 0;

        virtual auto GetTypeInfo() const -> TypeInfo = 0;
    };
}
