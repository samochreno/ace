#pragma once

#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace
{
    class ICallableSymbol;

    class IParamVarSymbol : public virtual IVarSymbol
    {
    public:
        virtual ~IParamVarSymbol() = default;

        virtual auto BindParent(ICallableSymbol* const parent) -> void = 0;
        virtual auto GetParent() const -> ICallableSymbol* = 0;
    };
}
