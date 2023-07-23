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
        llvm::Value* Value{};
        ITypeSymbol* TypeSymbol{};
    };
}
