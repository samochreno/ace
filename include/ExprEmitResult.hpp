#pragma once

#include <vector>

#include "Symbols/Types/TypeSymbol.hpp"
#include "ExprDropData.hpp"

namespace llvm
{
    class Value;
}

namespace Ace
{
    struct ExprEmitResult
    {
        llvm::Value* Value{};
        std::vector<ExprDropData> Tmps{};
    };
}
