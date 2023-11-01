#pragma once

#include "Symbols/All.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto CreateMissingFunctionBlockError(
        FunctionSymbol* const functionSymbol
    ) -> DiagnosticGroup;
}
