#pragma once

#include <type_traits>

#include "BoundNode/Base.hpp"
#include "Asserts.hpp"
#include "Symbols/TypedSymbol.hpp"

namespace Ace::BoundNode
{
    template<typename TSymbol>
    class ITyped : public virtual BoundNode::IBase
    {
    public:
        static_assert(std::is_base_of_v<ITypedSymbol, TSymbol>);

        virtual ~ITyped() = default;

        virtual auto GetSymbol() const -> TSymbol* = 0;
    };
}
