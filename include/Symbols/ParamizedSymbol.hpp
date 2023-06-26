#pragma once

#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    class IParamizedSymbol : public virtual ISymbol
    {
    public:
        virtual ~IParamizedSymbol() = default;

        virtual auto CollectParams()     const -> std::vector<IParamVarSymbol*> = 0;
        virtual auto CollectParamTypes() const -> std::vector<ITypeSymbol*> final;
    };
}
