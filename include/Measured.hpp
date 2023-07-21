#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>

namespace Ace
{
    template<typename T>
    struct Measured
    {
        Measured() = default;
        Measured(
            const T& value,
            const size_t length
        ) : Value{ value },
            Length{ length }
        {
        }
        Measured(
            T&& value,
            const size_t length
        ) : Value{ std::move(value) },
            Length{ length }
        {
        }
        Measured(
            const T& value,
            const ptrdiff_t length
        ) : Value{ value },
            Length{ static_cast<size_t>(length) }
        {
        }
        Measured(
            T&& value,
            const ptrdiff_t length
        ) : Value{ std::move(value) },
            Length{ static_cast<size_t>(length) }
        {
        }

        T Value{};
        size_t Length{};
    };
}
