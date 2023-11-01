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

    struct ControlFlowNode
    {
        ControlFlowKind Kind{};
        LabelSymbol* LabelSymbol{};
    };

    struct ControlFlowGraph
    {
        std::vector<ControlFlowNode> Nodes{};
    };
}
