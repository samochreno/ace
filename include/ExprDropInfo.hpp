#pragma once

#include "Symbols/Types/TypeSymbol.hpp"

namespace llvm
{
    class Value;
}

namespace Ace
{
    struct ExprDropInfo
    {
        llvm::Value* Value{};
        ITypeSymbol* TypeSymbol{};
    };
}
