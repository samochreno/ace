#pragma once

#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    class INominalTypeSymbol : public virtual ITypeSymbol
    {
    public:
        virtual ~INominalTypeSymbol() = default;
    };
}
