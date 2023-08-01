#pragma once

#include <type_traits>

#include "BoundNodes/BoundNode.hpp"
#include "Assert.hpp"
#include "Symbols/TypedSymbol.hpp"

namespace Ace
{
    template<typename TSymbol>
    class ITypedBoundNode : public virtual IBoundNode
    {
        static_assert(std::is_base_of_v<ITypedSymbol, TSymbol>);

    public:
        virtual ~ITypedBoundNode() = default;

        virtual auto GetSymbol() const -> TSymbol* = 0;
    };
}
