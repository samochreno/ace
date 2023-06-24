#include "BoundNode/Base.hpp"

#include "Compilation.hpp"

namespace Ace::BoundNode
{
    auto IBase::GetCompilation() const -> const Compilation*
    {
        return GetScope()->GetCompilation();
    }
}
