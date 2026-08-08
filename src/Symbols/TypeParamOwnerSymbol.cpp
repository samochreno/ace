#include "Symbols/TypeParamOwnerSymbol.hpp"

#include "Scope.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"

namespace Ace
{
    auto ITypeParamOwnerSymbol::CollectOwnedTypeParams() const
        -> std::vector<TypeParamTypeSymbol*>
    {
        return GetBodyScope()->CollectTypeParams();
    }
}
