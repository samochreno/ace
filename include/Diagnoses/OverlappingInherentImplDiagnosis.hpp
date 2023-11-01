#pragma once

#include "Compilation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto DiagnoseOverlappingInherentImpls(
        Compilation* const compilation
    ) -> Diagnosed<void>;
}
