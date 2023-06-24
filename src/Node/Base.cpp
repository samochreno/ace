#include "Node/Base.hpp"

#include "Compilation.hpp"

namespace Ace::Node
{
    auto IBase::GetCompilation() const -> const Compilation*
    {
        return GetScope()->GetCompilation();
    }
}
