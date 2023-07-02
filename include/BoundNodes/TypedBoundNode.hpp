#pragma once

#include <type_traits>

#include "BoundNodes/BoundNode.hpp"
#include "Asserts.hpp"
#include "Symbols/TypedSymbol.hpp"

namespace Ace
{
    template<typename TSymbol>
    class ITypedBoundNode : public virtual IBoundNode
    {
    public:
        static_assert(std::is_base_of_v<ITypedSymbol, TSymbol>);

        virtual ~ITypedBoundNode() = default;

        virtual auto GetSymbol() const -> TSymbol* = 0;
    };
}
