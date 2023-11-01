#pragma once

#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    class ISizedTypeSymbol : public virtual ITypeSymbol
    {
    public:
        virtual ~ISizedTypeSymbol() = default;
    };
}
