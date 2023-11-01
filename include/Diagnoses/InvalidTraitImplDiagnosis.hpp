#pragma once

#include "Compilation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto DiagnoseInvalidTraitImpls(
        Compilation* const compilation
    ) -> Diagnosed<void>;
}
