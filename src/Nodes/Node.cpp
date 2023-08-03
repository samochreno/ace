#include "Nodes/Node.hpp"

#include "Compilation.hpp"

namespace Ace
{
    auto INode::GetCompilation() const -> Compilation*
    {
        return GetScope()->GetCompilation();
    }
}
