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

    inline auto operator&(const SymbolKind& t_lhs, const SymbolKind& t_rhs) -> uint16_t
    {
        return static_cast<uint16_t>(t_lhs) & static_cast<uint16_t>(t_rhs);
    }

    auto GetSymbolCreationOrder(const SymbolKind& t_kind) -> int8_t;
}
