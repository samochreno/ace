#pragma once

#include <vector>

#include "Symbol/Type/Base.hpp"
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
            llvm::Value* const t_value,
            const std::vector<ExprDropData>& t_temporaries
        ) : Value{ t_value },
            Temporaries{ t_temporaries }
        {
        }

        llvm::Value* Value{};
        std::vector<ExprDropData> Temporaries{};
    };
}
