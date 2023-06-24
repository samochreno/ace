#pragma once

#include <vector>

#include "Asserts.hpp"

namespace Ace
{
    template<typename TTarget, typename TOriginal>
    inline auto DynamicCastFilter(const std::vector<TOriginal>& t_vec) -> std::vector<TTarget>
    {
        std::vector<TTarget> vec{};

        std::for_each(begin(t_vec), end(t_vec),
        [&](const TOriginal& t_element)
        {
            auto target = dynamic_cast<TTarget>(t_element);

            if (target)
            {
                vec.push_back(target);
            }
        });

        return vec;
    }

    auto IsInAlphabet(const char& t_character) -> bool;
    auto IsNumber    (const char& t_character) -> bool;

    auto TrimRight(std::string& t_value) -> std::string&;
    auto TrimLeft (std::string& t_value) -> std::string&;
    auto Trim     (std::string& t_value) -> std::string&;

    template<typename TIterator>
    auto Distance(TIterator t_begin, TIterator t_end) -> size_t
    {
        ACE_ASSERT((t_end - t_begin) >= 0);
        return static_cast<size_t>(t_end - t_begin);
    }
}
