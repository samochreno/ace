#pragma once

#include <vector>

#include "Symbols/BodyScopedSymbol.hpp"

namespace Ace
{
    class TypeParamTypeSymbol;

    class ITypeParamOwnerSymbol : public virtual IBodyScopedSymbol
    {
    public:
        virtual ~ITypeParamOwnerSymbol() = default;

        virtual auto CollectOwnedTypeParams() const -> std::vector<TypeParamTypeSymbol*> final;
    };
}
