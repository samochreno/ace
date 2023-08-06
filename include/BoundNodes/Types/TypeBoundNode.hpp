#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/BoundNode.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ITypeBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ITypeBoundNode() = default;

        virtual auto CreateTypeCheckedType(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ITypeBoundNode>> = 0;
        virtual auto CreateLoweredType(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ITypeBoundNode> = 0;

        virtual auto GetSymbol() const -> ITypeSymbol* = 0;
    };
}
