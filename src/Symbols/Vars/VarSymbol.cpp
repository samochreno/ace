#include "Symbols/Vars/VarSymbol.hpp"

#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    auto IVarSymbol::GetType() const -> ITypeSymbol*
    {
        return GetSizedType();
    }
}
