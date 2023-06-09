#pragma once

namespace Ace
{
    template<typename T>
    struct ParseData
    {
        ParseData()
            : Length{ 0 }
        {
        }

        template<typename T_ = T, std::enable_if<std::is_copy_constructible_v<T>>* = nullptr>
        ParseData(const T& t_value, const size_t& t_length)
            : Value{ t_value }, Length{ t_length }
        {
        }

        ParseData(T&& t_value, const size_t& t_length)
            : Value{ std::move(t_value) }, Length{ t_length }
        {
        }

        T Value;
        size_t Length;
    };
}
