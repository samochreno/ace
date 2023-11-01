#include "AnonymousIdent.hpp"

#include <string>

namespace Ace::AnonymousIdent
{
    static size_t Counter = 0;

    auto Concat() -> std::string
    {
        return {};
    }

    auto Create() -> std::string
    {
        return "?" + std::to_string(Counter++);
    }
}
