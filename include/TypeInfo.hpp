#pragma once

#include "ValueKind.hpp"

namespace Ace
{
    class ITypeSymbol;

    struct TypeInfo
    {
        ITypeSymbol* Symbol{};
        ValueKind ValueKind{};
    };
}
