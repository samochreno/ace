#pragma once

#include "ValueKind.hpp"

namespace Ace
{
    class ITypeSymbol;

    struct TypeInfo
    {
    public:
        TypeInfo(
            ITypeSymbol* const symbol,
            const ValueKind valueKind
        ) : Symbol{ symbol },
            ValueKind{ valueKind }
        {
        }

        ITypeSymbol* Symbol{};
        ValueKind ValueKind{};
    };
}
