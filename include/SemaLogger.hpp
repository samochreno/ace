#pragma once

#include <string>

namespace Ace
{
    class SemaLogger
    {
    public:
        template<typename T>
        auto Log(T&& t) -> void {}

        template<typename T, typename U>
        auto Log(T&& t, U&& u) -> void {}

        auto CreateString() const -> std::string
        {
            return {};
        }
    };
}
