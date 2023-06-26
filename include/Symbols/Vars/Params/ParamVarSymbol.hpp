#pragma once

#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace
{
    class IParamVarSymbol : public virtual IVarSymbol
    {
    public:
        virtual ~IParamVarSymbol() = default;
    };
}
