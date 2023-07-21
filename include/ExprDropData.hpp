#pragma once

#include "Symbols/Types/TypeSymbol.hpp"

namespace llvm
{
    class Value;
}

namespace Ace
{
    struct ExprDropData
    {
        ExprDropData(
            llvm::Value* const value,
            ITypeSymbol* const typeSymbol
        ) : Value{ value },
            TypeSymbol{ typeSymbol }
        {
        }

        llvm::Value* Value{};
        ITypeSymbol* TypeSymbol{};
    };
}
