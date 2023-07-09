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
            const T& t_value,
            const size_t t_length
        ) : Value{ t_value },
            Length{ t_length }
        {
        }
        Measured(
            T&& t_value,
            const size_t t_length
        ) : Value{ std::move(t_value) },
            Length{ t_length }
        {
        }
        Measured(
            const T& t_value,
            const ptrdiff_t t_length
        ) : Value{ t_value },
            Length{ static_cast<size_t>(t_length) }
        {
        }
        Measured(
            T&& t_value,
            const ptrdiff_t t_length
        ) : Value{ std::move(t_value) },
            Length{ static_cast<size_t>(t_length) }
        {
        }

        T Value{};
        size_t Length{};
    };
}
