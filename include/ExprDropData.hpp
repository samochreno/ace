#pragma once

#include "Symbol/Type/Base.hpp"

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
            Symbol::Type::IBase* const t_typeSymbol
        ) : Value{ t_value },
            TypeSymbol{ t_typeSymbol }
        {
        }

        llvm::Value* Value{};
        Symbol::Type::IBase* TypeSymbol{};
    };
}
