#pragma once

#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    auto DoPlaceholdersOverlap(
        ITypeSymbol* conversative,
        ITypeSymbol* speculative
    ) -> bool;
}
