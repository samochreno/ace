#pragma once

#include <vector>

#include "Symbols/LabelSymbol.hpp"

namespace Ace
{
    enum class ControlFlowKind
    {
        Label,
        Jump,
        ConditionalJump,
        Ret,
        Exit,
    };

    struct ControlFlowInstruction
    {
        ControlFlowKind Kind{};
        LabelSymbol* LabelSymbol{};
    };

    struct ControlFlowGraph
    {
        std::vector<ControlFlowInstruction> Instructions{};
    };
}
