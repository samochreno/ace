#pragma once

#include <cstdint>

#include "Diagnostics.hpp"

namespace Ace
{
    enum class SymbolKind
    {
        None,
        Module,
        Struct,
        Label,
        Function,
        StaticVariable,
        InstanceVariable,
        LocalVariable,
        ParameterVariable,
        FunctionTemplate,
        TypeTemplate,
        TypeAlias,
        TemplatedImpl,
        ImplTemplateParameter,
        TemplateParameter,
    };

    constexpr auto operator&(
        const SymbolKind& t_lhs,
        const SymbolKind& t_rhs
    ) -> bool;

    auto GetSymbolCreationOrder(const SymbolKind& t_kind) -> int8_t;
}
