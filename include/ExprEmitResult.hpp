#pragma once

#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "ExprDropInfo.hpp"

namespace llvm
{
    class Value;
}

namespace Ace
{
    struct ExprEmitResult
    {
        llvm::Value* Value{};
        std::vector<ExprDropInfo> Tmps{};
    };
}
