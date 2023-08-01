#pragma once

#include <cstdint>

#include "Diagnostic.hpp"

namespace Ace
{
    enum class SymbolKind
    {
        Module,
        Struct,
        Label,
        Function,
        StaticVar,
        InstanceVar,
        LocalVar,
        ParamVar,
        FunctionTemplate,
        TypeTemplate,
        TypeAlias,
        ImplTemplateParam,
        TemplateParam,
    };

    constexpr auto operator&(
        const SymbolKind lhs,
        const SymbolKind rhs
    ) -> bool;

    auto GetSymbolCreationOrder(const SymbolKind kind) -> int8_t;
}
