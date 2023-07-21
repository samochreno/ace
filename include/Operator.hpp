#pragma once

#include "SourceLocation.hpp"
#include "TokenKind.hpp"

namespace Ace
{
    struct Operator
    {
        SourceLocation SourceLocation{};
        TokenKind TokenKind{};
    };
}
