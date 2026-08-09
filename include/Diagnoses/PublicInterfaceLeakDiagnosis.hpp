#pragma once

#include "Diagnostic.hpp"

namespace Ace
{
    class Compilation;

    auto DiagnosePublicInterfaceLeaks(
        Compilation* const compilation
    ) -> Diagnosed<void>;
}
