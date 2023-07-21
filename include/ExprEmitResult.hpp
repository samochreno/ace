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
        ExprEmitResult(
            llvm::Value* const value,
            const std::vector<ExprDropData>& temporaries
        ) : Value{ value },
            Temporaries{ temporaries }
        {
        }

        llvm::Value* Value{};
        std::vector<ExprDropData> Temporaries{};
    };
}
