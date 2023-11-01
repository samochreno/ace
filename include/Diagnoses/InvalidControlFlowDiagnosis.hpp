#pragma once

#include "ControlFlow.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto DiagnoseInvalidControlFlow(
        const SrcLocation& srcLocation,
        const ControlFlowGraph& graph
    ) -> Diagnosed<void>;
}
