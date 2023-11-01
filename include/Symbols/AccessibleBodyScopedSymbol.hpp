#pragma once

#include <memory>

#include "Symbols/BodyScopedSymbol.hpp"

namespace Ace
{
    class IAccessibleBodyScopedSymbol : public virtual IBodyScopedSymbol
    {
    public:
        virtual ~IAccessibleBodyScopedSymbol() = default;
    };
}
