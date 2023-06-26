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
            llvm::Value* const t_value,
            ITypeSymbol* const t_typeSymbol
        ) : Value{ t_value },
            TypeSymbol{ t_typeSymbol }
        {
        }

        llvm::Value* Value{};
        ITypeSymbol* TypeSymbol{};
    };
}
