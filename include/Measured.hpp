#pragma once

#include <utility>
#include <type_traits>

namespace Ace
{
    template<typename T>
    struct Measured
    {
        Measured() = default;
        template<typename T_ = T, std::enable_if<std::is_copy_constructible_v<T>>* = nullptr>
        Measured(const T& t_value, const size_t& t_length)
            : Value{ t_value }, Length{ t_length }
        {
        }
        Measured(T&& t_value, const size_t& t_length)
            : Value{ std::move(t_value) }, Length{ t_length }
        {
        }

        T Value{};
        size_t Length{};
    };
}
