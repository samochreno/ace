#pragma once

#include "Symbols/Symbol.hpp"
#include "Symbols/TypedSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    class IVarSymbol :
        public virtual ISymbol,
        public virtual ITypedSymbol
    {
    public:
        virtual ~IVarSymbol() = default;

        auto GetType() const -> ITypeSymbol* final;

        virtual auto GetSizedType() const -> ISizedTypeSymbol* = 0;
    };
}
