#pragma once

#include "SrcLocation.hpp"
#include "TokenKind.hpp"

namespace Ace
{
    struct Op
    {
        SrcLocation SrcLocation{};
        TokenKind TokenKind{};
    };
}
