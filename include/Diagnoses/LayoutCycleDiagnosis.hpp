#pragma once

#include "Compilation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto DiagnoseLayoutCycles(
        Compilation* const compilation
    ) -> Diagnosed<void>;
}
