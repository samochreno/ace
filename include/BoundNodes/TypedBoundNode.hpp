#pragma once

#include <type_traits>

#include "BoundNodes/BoundNode.hpp"

namespace Ace
{
    template<typename TSymbol>
    class ITypedBoundNode : public virtual IBoundNode
    {
    public:
        virtual ~ITypedBoundNode() = default;

        virtual auto GetSymbol() const -> TSymbol* = 0;
    };
}
