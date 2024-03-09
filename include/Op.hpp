#pragma once

namespace Ace
{
    enum class Op
    {
        UnaryPlus,
        UnaryNegation,

        NOT,

        Multiplication,
        Division,

        Remainder,

        Addition,
        Subtraction,

        RightShift,
        LeftShift,

        LessThan,
        GreaterThan,
        LessThanEquals,
        GreaterThanEquals,

        Equals,
        NotEquals,

        AND,
        XOR,
        OR,

        Copy,
        Drop,
    };
}
