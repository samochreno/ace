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
            Symbol::Type::IBase* const t_symbol,
            const ValueKind& t_valueKind
        ) : Symbol{ t_symbol },
            ValueKind{ t_valueKind }
        {
        }

        Symbol::Type::IBase* Symbol{};
        ValueKind ValueKind{};
    };
}
