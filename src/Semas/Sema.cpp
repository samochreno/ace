#include "Semas/Sema.hpp"

#include <vector>

#include "Compilation.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    auto MonoCollector::Get() const -> const std::vector<IGenericSymbol*>&
    {
        return m_Placeholders;
    }

    auto ISema::GetCompilation() const -> Compilation*
    {
        return GetScope()->GetCompilation();
    }
}
