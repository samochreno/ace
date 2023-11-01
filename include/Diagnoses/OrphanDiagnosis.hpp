#pragma once

#include "Compilation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto DiagnoseOrphans(Compilation* const compilation) -> Diagnosed<void>;
}
