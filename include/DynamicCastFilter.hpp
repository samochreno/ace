#pragma once

#include <vector>
#include <algorithm>

#include "Assert.hpp"

namespace Ace
{
    template<typename TTarget, typename TOriginal>
    inline auto DynamicCastFilter(const std::vector<TOriginal>& inVec) -> std::vector<TTarget>
    {
        std::vector<TTarget> vec{};

        std::for_each(begin(inVec), end(inVec),
        [&](const TOriginal& element)
        {
            auto target = dynamic_cast<TTarget>(element);

            if (target)
            {
                vec.push_back(target);
            }
        });

        return vec;
    }
}
