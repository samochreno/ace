#pragma once

#include <cstdint>

#include "Error.hpp"

namespace Ace::Symbol
{
    enum class Kind
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
    };

    inline auto operator&(const Kind& t_lhs, const Kind& t_rhs) -> uint16_t
    {
        return static_cast<uint16_t>(t_lhs) & static_cast<uint16_t>(t_rhs);
    }

    auto GetCreationOrder(const Symbol::Kind& t_kind) -> int8_t;
}
