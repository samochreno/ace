#pragma once

#include <vector>

#include "Symbol/Type/Base.hpp"
#include "ExpressionDropData.hpp"

namespace llvm
{
    class Value;
}

namespace Ace
{
    struct ExpressionEmitResult
    {
        ExpressionEmitResult(
            llvm::Value* const t_value,
            const std::vector<ExpressionDropData>& t_temporaries
        ) : Value{ t_value },
            Temporaries{ t_temporaries }
        {
        }

        llvm::Value* Value{};
        std::vector<ExpressionDropData> Temporaries{};
    };
}
