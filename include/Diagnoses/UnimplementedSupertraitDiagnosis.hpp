#pragma once

#include "Compilation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto DiagnoseUnimplementedSupertraits(
        Compilation* const compilation
    ) -> Diagnosed<void>;
}
