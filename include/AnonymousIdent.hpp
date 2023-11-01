#pragma once

#include <string>

namespace Ace::AnonymousIdent
{
    auto Concat() -> std::string;
    template<typename... Args>
    auto Concat(const std::string& first, const Args&... rest) -> std::string
    {
        std::string value = first;

        if constexpr (sizeof...(rest) > 0) {
            value += "_";
        }

        value += Concat(rest...);
        return value;
    }

    auto Create() -> std::string;
    template<typename... Args>
    auto Create(const std::string& first, const Args&... rest) -> std::string
    {
        return Create() + "_" + Concat(first, rest...);
    }

}
