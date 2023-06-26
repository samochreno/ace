#pragma once

#include "Symbols/Symbol.hpp"
#include "Symbols/TypedSymbol.hpp"

namespace Ace
{
    class IVarSymbol : public virtual ISymbol, public virtual ITypedSymbol
    {
    public:
        virtual ~IVarSymbol() = default;
    };
}
