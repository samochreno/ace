#pragma once

#include <vector>

#include "Asserts.hpp"

namespace Ace
{
    template<typename TTarget, typename TOriginal>
    inline auto DynamicCastFilter(const std::vector<TOriginal>& t_vec) -> std::vector<TTarget>
    {
        std::vector<TTarget> vec{};

        std::for_each(begin(t_vec), end(t_vec), [&](const TOriginal& t_element)
        {
            auto target = dynamic_cast<TTarget>(t_element);

            if (target)
                vec.push_back(target);
        });

        return vec;
    }

    inline auto IsInAlphabet(const char& t_char) -> bool
    {
        return
            ((t_char >= 'a') && (t_char <= 'z')) ||
            ((t_char >= 'A') && (t_char <= 'Z'));
    }

    inline auto IsNumber(const char& t_char) -> bool
    {
        return (t_char >= '0') && (t_char <= '9');
    }

    inline auto TrimRight(std::string& t_value) -> std::string&
    {
        t_value.erase(t_value.find_last_not_of(' ') + 1);
        return t_value;
    }

    inline auto TrimLeft(std::string& t_value) -> std::string&
    {
        t_value.erase(0, t_value.find_first_not_of(' '));
        return t_value;
    }

    inline auto Trim(std::string& t_value) -> std::string&
    {
        TrimLeft(t_value);
        TrimRight(t_value);
        return t_value;
    }

    template<typename TIterator>
    auto Distance(TIterator t_begin, TIterator t_end) -> size_t
    {
        ACE_ASSERT((t_end - t_begin) >= 0);
        return static_cast<size_t>(t_end - t_begin);
    }
}
