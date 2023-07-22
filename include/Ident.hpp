#pragma once

#include <string>

#include "SrcLocation.hpp"

namespace Ace
{
    struct Ident
    {
        SrcLocation SrcLocation{};
        std::string String{};
    };
}
