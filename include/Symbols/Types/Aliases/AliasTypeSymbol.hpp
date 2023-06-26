#pragma once

#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    class IAliasTypeSymbol : public virtual ITypeSymbol
    {
    public:
        virtual ~IAliasTypeSymbol() = default;

        virtual auto GetAliasedType() const -> ITypeSymbol* = 0;
    };
}
