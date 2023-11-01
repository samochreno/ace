#pragma once

#include "Compilation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto DiagnoseConcreteConstraints(
        Compilation* const compilation
    ) -> Diagnosed<void>;
}
