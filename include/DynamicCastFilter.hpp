#pragma once

#include <vector>
#include <algorithm>

#include "Assert.hpp"

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
}
