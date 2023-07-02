#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    class ITypeBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ITypeBoundNode() = default;

        virtual auto GetOrCreateTypeCheckedType(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const ITypeBoundNode>>> = 0;
        virtual auto GetOrCreateLoweredType(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const ITypeBoundNode>> = 0;

        virtual auto GetSymbol() const -> ITypeSymbol* = 0;
    };
}
