#pragma once

#include <cstdint>

namespace Ace
{
    enum class LiteralKind : uint16_t
    {
        Int8    = 1 << 0,
        Int16   = 1 << 1,
        Int32   = 1 << 2,
        Int64   = 1 << 3,

        UInt8   = 1 << 4,
        UInt16  = 1 << 5,
        UInt32  = 1 << 6,
        UInt64  = 1 << 7,

        Int     = 1 << 8,

        Float32 = 1 << 9,
        Float64 = 1 << 10,

        String  = 1 << 11,
        True    = 1 << 12,
        False   = 1 << 13,

        IntMask     = Int8 | Int16 | Int32 | Int64 | UInt8 | UInt16 | UInt32 | UInt64 | Int,
        FloatMask   = Float32 | Float64,
        NumberMask  = IntMask | FloatMask,
        BoolMask    = True | False,
    };

    inline constexpr auto operator&(
        const LiteralKind t_lhs,
        const LiteralKind t_rhs
    ) -> bool
    {
        return
            (static_cast<uint16_t>(t_lhs) & static_cast<uint16_t>(t_rhs)) != 0;
    }
}
