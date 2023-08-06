#pragma once

#include "Diagnostic.hpp"
#include "Symbols/LabelSymbol.hpp"

namespace Ace
{
    class LabelSymbol;

    enum class CFANodeKind
    {
        Label,
        Jump,
        ConditionalJump,
        Return,
        Exit,
    };

    struct CFANode
    {
        CFANodeKind Kind{};
        LabelSymbol* LabelSymbol{};
    };

    struct CFAGraph
    {
        std::vector<CFANode> Nodes{};
    };

    auto CFA(
        const SrcLocation& srcLocation,
        const CFAGraph& graph
    ) -> Diagnosed<void>;
}
