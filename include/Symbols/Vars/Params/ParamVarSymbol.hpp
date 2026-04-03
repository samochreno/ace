#pragma once

#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace
{
    class ICallableSymbol;

    class IParamVarSymbol : public virtual IVarSymbol
    {
    public:
        virtual ~IParamVarSymbol() = default;

        virtual auto SetParentCallable(ICallableSymbol* const callable) -> void = 0;
        virtual auto GetParentCallable() const -> ICallableSymbol* = 0;
    };
}
