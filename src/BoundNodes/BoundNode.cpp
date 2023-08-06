#include "BoundNodes/BoundNode.hpp"

#include "Compilation.hpp"

namespace Ace
{
    auto IBoundNode::GetCompilation() const -> Compilation*
    {
        return GetScope()->GetCompilation();
    }
}
