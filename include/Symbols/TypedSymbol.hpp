#pragma once

#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    class ITypedSymbol : public virtual ISymbol
    {
    public:
        virtual ~ITypedSymbol() = default;

        virtual auto GetType() const -> ITypeSymbol* = 0;
    };
}
