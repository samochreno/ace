#pragma once

#include "ValueKind.hpp"

namespace Ace::Symbol::Type
{
    class IBase;
}

namespace Ace
{
    struct TypeInfo
    {
    public:
        TypeInfo(
            ITypeSymbol* const t_symbol,
            const ValueKind t_valueKind
        ) : Symbol{ t_symbol },
            ValueKind{ t_valueKind }
        {
        }

        ITypeSymbol* Symbol{};
        Ace::ValueKind ValueKind{};
    };
}
