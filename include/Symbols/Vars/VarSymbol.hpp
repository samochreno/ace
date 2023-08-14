#pragma once

#include "Symbols/Symbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    class IVarSymbol : public virtual ISymbol
    {
    public:
        virtual ~IVarSymbol() = default;

        virtual auto GetType() const -> ISizedTypeSymbol* = 0;
    };
}
