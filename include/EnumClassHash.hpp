#pragma once

#include <cstddef>

namespace Ace
{
    struct EnumClassHash
    {
        template<typename T>
        size_t operator()(T t) const
        {
            return static_cast<size_t>(t);
        }
    };
}
