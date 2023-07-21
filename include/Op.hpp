#pragma once

#include "SourceLocation.hpp"
#include "TokenKind.hpp"

namespace Ace
{
    struct Op
    {
        SourceLocation SourceLocation{};
        TokenKind TokenKind{};
    };
}
