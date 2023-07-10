#pragma once

#include <string>

#include "SourceLocation.hpp"

namespace Ace
{
    struct Identifier
    {
        SourceLocation SourceLocation{};
        std::string String{};
    };
}
