#include "Std.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Compilation.hpp"
#include "FileBuffer.hpp"

static inline constexpr const char* bool_ = R"(
pub ::
Bool: struct { 
}
)";

static inline constexpr const char* float_ = R"(
pub ::
Float32: struct { 
}

pub extern ::
f32_from_i8(value: i8): f32;

pub extern ::
f32_from_i16(value: i16): f32;

pub extern ::
f32_from_i32(value: i32): f32;

pub extern ::
f32_from_i64(value: i64): f32;

pub extern ::
f32_from_u8(value: u8): f32;

pub extern ::
f32_from_u16(value: u16): f32;

pub extern ::
f32_from_u32(value: u32): f32;

pub extern ::
f32_from_u64(value: u64): f32;

pub extern ::
f32_from_int(value: int): f32;

pub extern ::
f32_from_f64(value: f64): f32;

pub extern ::
f32_unary_negation(value: f32): f32;

pub extern ::
f32_multiplication(lhs: f32, rhs: f32): f32;

pub extern ::
f32_division(lhs: f32, rhs: f32): f32;

pub extern ::
f32_remainder(lhs: f32, rhs: f32): f32;

pub extern ::
f32_addition(lhs: f32, rhs: f32): f32;

pub extern ::
f32_subtraction(lhs: f32, rhs: f32): f32;

pub extern ::
f32_less_than(lhs: f32, rhs: f32): bool;

pub extern ::
f32_greater_than(lhs: f32, rhs: f32): bool;

pub extern ::
f32_less_than_equals(lhs: f32, rhs: f32): bool;

pub extern ::
f32_greater_than_equals(lhs: f32, rhs: f32): bool;

pub extern ::
f32_equals(lhs: f32, rhs: f32): bool;

pub extern ::
f32_not_equals(lhs: f32, rhs: f32): bool;

impl Minus for f32 {
    self ::
    minus(): Self {
        ret f32_unary_negation(self);
    }
}

impl Multiply[Self] for f32 {
    self ::
    multiply(other: &Self): Self {
        ret f32_multiplication(self, other);
    }
}

impl Divide[Self] for f32 {
    self ::
    divide(other: &Self): Self {
        ret f32_division(self, other);
    }
}

impl Remainder[Self] for f32 {
    self ::
    remainder(other: &Self): Self {
        ret f32_remainder(self, other);
    }
}

impl Add[Self] for f32 {
    self ::
    add(other: &Self): Self {
        ret f32_addition(self, other);
    }
}

impl Subtract[Self] for f32 {
    self ::
    subtract(other: &Self): Self {
        ret f32_subtraction(self, other);
    }
}

pub ::
Float64: struct { 
}

pub extern ::
f64_from_i8(value: i8): f64;

pub extern ::
f64_from_i16(value: i16): f64;

pub extern ::
f64_from_i32(value: i32): f64;

pub extern ::
f64_from_i64(value: i64): f64;

pub extern ::
f64_from_u8(value: u8): f64;

pub extern ::
f64_from_u16(value: u16): f64;

pub extern ::
f64_from_u32(value: u32): f64;

pub extern ::
f64_from_u64(value: u64): f64;

pub extern ::
f64_from_int(value: int): f64;

pub extern ::
f64_from_f32(value: f32): f64;

pub extern ::
f64_unary_negation(value: f64): f64;

pub extern ::
f64_multiplication(lhs: f64, rhs: f64): f64;

pub extern ::
f64_division(lhs: f64, rhs: f64): f64;

pub extern ::
f64_remainder(lhs: f64, rhs: f64): f64;

pub extern ::
f64_addition(lhs: f64, rhs: f64): f64;

pub extern ::
f64_subtraction(lhs: f64, rhs: f64): f64;

pub extern ::
f64_less_than(lhs: f64, rhs: f64): bool;

pub extern ::
f64_greater_than(lhs: f64, rhs: f64): bool;

pub extern ::
f64_less_than_equals(lhs: f64, rhs: f64): bool;

pub extern ::
f64_greater_than_equals(lhs: f64, rhs: f64): bool;

pub extern ::
f64_equals(lhs: f64, rhs: f64): bool;

pub extern ::
f64_not_equals(lhs: f64, rhs: f64): bool;

impl Minus for f64 {
    self ::
    minus(): Self {
        ret f64_unary_negation(self);
    }
}

impl Multiply[Self] for f64 {
    self ::
    multiply(other: &Self): Self {
        ret f64_multiplication(self, other);
    }
}

impl Divide[Self] for f64 {
    self ::
    divide(other: &Self): Self {
        ret f64_division(self, other);
    }
}

impl Remainder[Self] for f64 {
    self ::
    remainder(other: &Self): Self {
        ret f64_remainder(self, other);
    }
}

impl Add[Self] for f64 {
    self ::
    add(other: &Self): Self {
        ret f64_addition(self, other);
    }
}

impl Subtract[Self] for f64 {
    self ::
    subtract(other: &Self): Self {
        ret f64_subtraction(self, other);
    }
}
)";

static inline constexpr const char* int_ = R"(
pub ::
Int8: struct {
}

pub extern ::
i8_from_i16(value: i16): i8;

pub extern ::
i8_from_i32(value: i32): i8;

pub extern ::
i8_from_i64(value: i64): i8;

pub extern ::
i8_from_u8(value: u8): i8;

pub extern ::
i8_from_u16(value: u16): i8;

pub extern ::
i8_from_u32(value: u32): i8;

pub extern ::
i8_from_u64(value: u64): i8;

pub extern ::
i8_from_int(value: int): i8;

pub extern ::
i8_from_f32(value: f32): i8;

pub extern ::
i8_from_f64(value: f64): i8;

pub extern ::
i8_unary_negation(value: i8): i8;

pub extern ::
i8_NOT(value: i8): i8;

pub extern ::
i8_multiplication(lhs: i8, rhs: i8): i8;

pub extern ::
i8_division(lhs: i8, rhs: i8): i8;

pub extern ::
i8_remainder(lhs: i8, rhs: i8): i8;

pub extern ::
i8_addition(lhs: i8, rhs: i8): i8;

pub extern ::
i8_subtraction(lhs: i8, rhs: i8): i8;

pub extern ::
i8_right_shift(lhs: i8, rhs: i8): i8;

pub extern ::
i8_left_shift(lhs: i8, rhs: i8): i8;

pub extern ::
i8_less_than(lhs: i8, rhs: i8): bool;

pub extern ::
i8_greater_than(lhs: i8, rhs: i8): bool;

pub extern ::
i8_less_than_equals(lhs: i8, rhs: i8): bool;

pub extern ::
i8_greater_than_equals(lhs: i8, rhs: i8): bool;

pub extern ::
i8_equals(lhs: i8, rhs: i8): bool;

pub extern ::
i8_not_equals(lhs: i8, rhs: i8): bool;

pub extern ::
i8_AND(lhs: i8, rhs: i8): i8;

pub extern ::
i8_XOR(lhs: i8, rhs: i8): i8;

pub extern ::
i8_OR(lhs: i8, rhs: i8): i8;

impl Minus for i8 {
    self ::
    minus(): Self {
        ret i8_unary_negation(self);
    }
}

impl Multiply[Self] for i8 {
    self ::
    multiply(other: &Self): Self {
        ret i8_multiplication(self, other);
    }
}

impl Divide[Self] for i8 {
    self ::
    divide(other: &Self): Self {
        ret i8_division(self, other);
    }
}

impl Remainder[Self] for i8 {
    self ::
    remainder(other: &Self): Self {
        ret i8_remainder(self, other);
    }
}

impl Add[Self] for i8 {
    self ::
    add(other: &Self): Self {
        ret i8_addition(self, other);
    }
}

impl Subtract[Self] for i8 {
    self ::
    subtract(other: &Self): Self {
        ret i8_subtraction(self, other);
    }
}

impl Equal[Self] for i8 {
    self ::
    equal(other: &Self): bool {
        ret i8_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret i8_not_equals(self, other);
    }
}

impl AND[Self] for i8 {
    self ::
    and(other: &Self): Self {
        ret i8_AND(self, other);
    }
}

impl OR[Self] for i8 {
    self ::
    or(other: &Self): Self {
        ret i8_OR(self, other);
    }
}

impl XOR[Self] for i8 {
    self ::
    xor(other: &Self): Self {
        ret i8_XOR(self, other);
    }
}

pub ::
Int16: struct {
}

pub extern ::
i16_from_i8(value: i8): i16;

pub extern ::
i16_from_i32(value: i32): i16;

pub extern ::
i16_from_i64(value: i64): i16;

pub extern ::
i16_from_u8(value: u8): i16;

pub extern ::
i16_from_u16(value: u16): i16;

pub extern ::
i16_from_u32(value: u32): i16;

pub extern ::
i16_from_u64(value: u64): i16;

pub extern ::
i16_from_int(value: int): i16;

pub extern ::
i16_from_f32(value: f32): i16;

pub extern ::
i16_from_f64(value: f64): i16;

pub extern ::
i16_unary_negation(value: i16): i16;

pub extern ::
i16_NOT(value: i16): i16;

pub extern ::
i16_multiplication(lhs: i16, rhs: i16): i16;

pub extern ::
i16_division(lhs: i16, rhs: i16): i16;

pub extern ::
i16_remainder(lhs: i16, rhs: i16): i16;

pub extern ::
i16_addition(lhs: i16, rhs: i16): i16;

pub extern ::
i16_subtraction(lhs: i16, rhs: i16): i16;

pub extern ::
i16_right_shift(lhs: i16, rhs: i16): i16;

pub extern ::
i16_left_shift(lhs: i16, rhs: i16): i16;

pub extern ::
i16_less_than(lhs: i16, rhs: i16): bool;

pub extern ::
i16_greater_than(lhs: i16, rhs: i16): bool;

pub extern ::
i16_less_than_equals(lhs: i16, rhs: i16): bool;

pub extern ::
i16_greater_than_equals(lhs: i16, rhs: i16): bool;

pub extern ::
i16_equals(lhs: i16, rhs: i16): bool;

pub extern ::
i16_not_equals(lhs: i16, rhs: i16): bool;

pub extern ::
i16_AND(lhs: i16, rhs: i16): i16;

pub extern ::
i16_XOR(lhs: i16, rhs: i16): i16;

pub extern ::
i16_OR(lhs: i16, rhs: i16): i16;

impl Minus for i16 {
    self ::
    minus(): Self {
        ret i16_unary_negation(self);
    }
}

impl Multiply[Self] for i16 {
    self ::
    multiply(other: &Self): Self {
        ret i16_multiplication(self, other);
    }
}

impl Divide[Self] for i16 {
    self ::
    divide(other: &Self): Self {
        ret i16_division(self, other);
    }
}

impl Remainder[Self] for i16 {
    self ::
    remainder(other: &Self): Self {
        ret i16_remainder(self, other);
    }
}

impl Add[Self] for i16 {
    self ::
    add(other: &Self): Self {
        ret i16_addition(self, other);
    }
}

impl Subtract[Self] for i16 {
    self ::
    subtract(other: &Self): Self {
        ret i16_subtraction(self, other);
    }
}

impl Equal[Self] for i16 {
    self ::
    equal(other: &Self): bool {
        ret i16_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret i16_not_equals(self, other);
    }
}

impl AND[Self] for i16 {
    self ::
    and(other: &Self): Self {
        ret i16_AND(self, other);
    }
}

impl OR[Self] for i16 {
    self ::
    or(other: &Self): Self {
        ret i16_OR(self, other);
    }
}

impl XOR[Self] for i16 {
    self ::
    xor(other: &Self): Self {
        ret i16_XOR(self, other);
    }
}

pub ::
Int32: struct {
}

pub extern ::
i32_from_i8(value: i8): i32;

pub extern ::
i32_from_i16(value: i16): i32;

pub extern ::
i32_from_i64(value: i64): i32;

pub extern ::
i32_from_u8(value: u8): i32;

pub extern ::
i32_from_u16(value: u16): i32;

pub extern ::
i32_from_u32(value: u32): i32;

pub extern ::
i32_from_u64(value: u64): i32;

pub extern ::
i32_from_int(value: int): i32;

pub extern ::
i32_from_f32(value: f32): i32;

pub extern ::
i32_from_f64(value: f64): i32;

pub extern ::
i32_unary_negation(value: i32): i32;

pub extern ::
i32_NOT(value: i32): i32;

pub extern ::
i32_multiplication(lhs: i32, rhs: i32): i32;

pub extern ::
i32_division(lhs: i32, rhs: i32): i32;

pub extern ::
i32_remainder(lhs: i32, rhs: i32): i32;

pub extern ::
i32_addition(lhs: i32, rhs: i32): i32;

pub extern ::
i32_subtraction(lhs: i32, rhs: i32): i32;

pub extern ::
i32_right_shift(lhs: i32, rhs: i32): i32;

pub extern ::
i32_left_shift(lhs: i32, rhs: i32): i32;

pub extern ::
i32_less_than(lhs: i32, rhs: i32): bool;

pub extern ::
i32_greater_than(lhs: i32, rhs: i32): bool;

pub extern ::
i32_less_than_equals(lhs: i32, rhs: i32): bool;

pub extern ::
i32_greater_than_equals(lhs: i32, rhs: i32): bool;

pub extern ::
i32_equals(lhs: i32, rhs: i32): bool;

pub extern ::
i32_not_equals(lhs: i32, rhs: i32): bool;

pub extern ::
i32_AND(lhs: i32, rhs: i32): i32;

pub extern ::
i32_XOR(lhs: i32, rhs: i32): i32;

pub extern ::
i32_OR(lhs: i32, rhs: i32): i32;

impl Minus for i32 {
    self ::
    minus(): Self {
        ret i32_unary_negation(self);
    }
}

impl Multiply[Self] for i32 {
    self ::
    multiply(other: &Self): Self {
        ret i32_multiplication(self, other);
    }
}

impl Divide[Self] for i32 {
    self ::
    divide(other: &Self): Self {
        ret i32_division(self, other);
    }
}

impl Remainder[Self] for i32 {
    self ::
    remainder(other: &Self): Self {
        ret i32_remainder(self, other);
    }
}

impl Add[Self] for i32 {
    self ::
    add(other: &Self): Self {
        ret i32_addition(self, other);
    }
}

impl Subtract[Self] for i32 {
    self ::
    subtract(other: &Self): Self {
        ret i32_subtraction(self, other);
    }
}

impl Equal[Self] for i32 {
    self ::
    equal(other: &Self): bool {
        ret i32_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret i32_not_equals(self, other);
    }
}

impl AND[Self] for i32 {
    self ::
    and(other: &Self): Self {
        ret i32_AND(self, other);
    }
}

impl OR[Self] for i32 {
    self ::
    or(other: &Self): Self {
        ret i32_OR(self, other);
    }
}

impl XOR[Self] for i32 {
    self ::
    xor(other: &Self): Self {
        ret i32_XOR(self, other);
    }
}

pub ::
Int64: struct {
}

pub extern ::
i64_from_i8(value: i8): i64;

pub extern ::
i64_from_i16(value: i16): i64;

pub extern ::
i64_from_i32(value: i32): i64;

pub extern ::
i64_from_u8(value: u8): i64;

pub extern ::
i64_from_u16(value: u16): i64;

pub extern ::
i64_from_u32(value: u32): i64;

pub extern ::
i64_from_u64(value: u64): i64;

pub extern ::
i64_from_int(value: int): i64;

pub extern ::
i64_from_f32(value: f32): i64;

pub extern ::
i64_from_f64(value: f64): i64;

pub extern ::
i64_unary_negation(value: i64): i64;

pub extern ::
i64_NOT(value: i64): i64;

pub extern ::
i64_multiplication(lhs: i64, rhs: i64): i64;

pub extern ::
i64_division(lhs: i64, rhs: i64): i64;

pub extern ::
i64_remainder(lhs: i64, rhs: i64): i64;

pub extern ::
i64_addition(lhs: i64, rhs: i64): i64;

pub extern ::
i64_subtraction(lhs: i64, rhs: i64): i64;

pub extern ::
i64_right_shift(lhs: i64, rhs: i64): i64;

pub extern ::
i64_left_shift(lhs: i64, rhs: i64): i64;

pub extern ::
i64_less_than(lhs: i64, rhs: i64): bool;

pub extern ::
i64_greater_than(lhs: i64, rhs: i64): bool;

pub extern ::
i64_less_than_equals(lhs: i64, rhs: i64): bool;

pub extern ::
i64_greater_than_equals(lhs: i64, rhs: i64): bool;

pub extern ::
i64_equals(lhs: i64, rhs: i64): bool;

pub extern ::
i64_not_equals(lhs: i64, rhs: i64): bool;

pub extern ::
i64_AND(lhs: i64, rhs: i64): i64;

pub extern ::
i64_XOR(lhs: i64, rhs: i64): i64;

pub extern ::
i64_OR(lhs: i64, rhs: i64): i64;

impl Minus for i64 {
    self ::
    minus(): Self {
        ret i64_unary_negation(self);
    }
}

impl Multiply[Self] for i64 {
    self ::
    multiply(other: &Self): Self {
        ret i64_multiplication(self, other);
    }
}

impl Divide[Self] for i64 {
    self ::
    divide(other: &Self): Self {
        ret i64_division(self, other);
    }
}

impl Remainder[Self] for i64 {
    self ::
    remainder(other: &Self): Self {
        ret i64_remainder(self, other);
    }
}

impl Add[Self] for i64 {
    self ::
    add(other: &Self): Self {
        ret i64_addition(self, other);
    }
}

impl Subtract[Self] for i64 {
    self ::
    subtract(other: &Self): Self {
        ret i64_subtraction(self, other);
    }
}

impl Equal[Self] for i64 {
    self ::
    equal(other: &Self): bool {
        ret i64_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret i64_not_equals(self, other);
    }
}

impl AND[Self] for i64 {
    self ::
    and(other: &Self): Self {
        ret i64_AND(self, other);
    }
}

impl OR[Self] for i64 {
    self ::
    or(other: &Self): Self {
        ret i64_OR(self, other);
    }
}

impl XOR[Self] for i64 {
    self ::
    xor(other: &Self): Self {
        ret i64_XOR(self, other);
    }
}

pub ::
UInt8: struct {
}

pub extern ::
u8_from_i8(value: i8): u8;

pub extern ::
u8_from_i16(value: i16): u8;

pub extern ::
u8_from_i32(value: i32): u8;

pub extern ::
u8_from_i64(value: i64): u8;

pub extern ::
u8_from_u16(value: u16): u8;

pub extern ::
u8_from_u32(value: u32): u8;

pub extern ::
u8_from_u64(value: u64): u8;

pub extern ::
u8_from_int(value: int): u8;

pub extern ::
u8_from_f32(value: f32): u8;

pub extern ::
u8_from_f64(value: f64): u8;

pub extern ::
u8_unary_negation(value: u8): u8;

pub extern ::
u8_NOT(value: u8): u8;

pub extern ::
u8_multiplication(lhs: u8, rhs: u8): u8;

pub extern ::
u8_division(lhs: u8, rhs: u8): u8;

pub extern ::
u8_remainder(lhs: u8, rhs: u8): u8;

pub extern ::
u8_addition(lhs: u8, rhs: u8): u8;

pub extern ::
u8_subtraction(lhs: u8, rhs: u8): u8;

pub extern ::
u8_right_shift(lhs: u8, rhs: u8): u8;

pub extern ::
u8_left_shift(lhs: u8, rhs: u8): u8;

pub extern ::
u8_less_than(lhs: u8, rhs: u8): bool;

pub extern ::
u8_greater_than(lhs: u8, rhs: u8): bool;

pub extern ::
u8_less_than_equals(lhs: u8, rhs: u8): bool;

pub extern ::
u8_greater_than_equals(lhs: u8, rhs: u8): bool;

pub extern ::
u8_equals(lhs: u8, rhs: u8): bool;

pub extern ::
u8_not_equals(lhs: u8, rhs: u8): bool;

pub extern ::
u8_AND(lhs: u8, rhs: u8): u8;

pub extern ::
u8_XOR(lhs: u8, rhs: u8): u8;

pub extern ::
u8_OR(lhs: u8, rhs: u8): u8;

impl Minus for u8 {
    self ::
    minus(): Self {
        ret u8_unary_negation(self);
    }
}

impl Multiply[Self] for u8 {
    self ::
    multiply(other: &Self): Self {
        ret u8_multiplication(self, other);
    }
}

impl Divide[Self] for u8 {
    self ::
    divide(other: &Self): Self {
        ret u8_division(self, other);
    }
}

impl Remainder[Self] for u8 {
    self ::
    remainder(other: &Self): Self {
        ret u8_remainder(self, other);
    }
}

impl Add[Self] for u8 {
    self ::
    add(other: &Self): Self {
        ret u8_addition(self, other);
    }
}

impl Subtract[Self] for u8 {
    self ::
    subtract(other: &Self): Self {
        ret u8_subtraction(self, other);
    }
}

impl Equal[Self] for u8 {
    self ::
    equal(other: &Self): bool {
        ret u8_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret u8_not_equals(self, other);
    }
}

impl AND[Self] for u8 {
    self ::
    and(other: &Self): Self {
        ret u8_AND(self, other);
    }
}

impl OR[Self] for u8 {
    self ::
    or(other: &Self): Self {
        ret u8_OR(self, other);
    }
}

impl XOR[Self] for u8 {
    self ::
    xor(other: &Self): Self {
        ret u8_XOR(self, other);
    }
}

pub ::
UInt16: struct { 
}

pub extern ::
u16_from_i8(value: i8): u16;

pub extern ::
u16_from_i16(value: i16): u16;

pub extern ::
u16_from_i32(value: i32): u16;

pub extern ::
u16_from_i64(value: i32): u16;

pub extern ::
u16_from_u8(value: u8): u16;

pub extern ::
u16_from_u32(value: u32): u16;

pub extern ::
u16_from_u64(value: u64): u16;

pub extern ::
u16_from_int(value: int): u16;

pub extern ::
u16_from_f32(value: f32): u16;

pub extern ::
u16_from_f64(value: f64): u16;

pub extern ::
u16_unary_negation(value: u16): u16;

pub extern ::
u16_NOT(value: u16): u16;

pub extern ::
u16_multiplication(lhs: u16, rhs: u16): u16;

pub extern ::
u16_division(lhs: u16, rhs: u16): u16;

pub extern ::
u16_remainder(lhs: u16, rhs: u16): u16;

pub extern ::
u16_addition(lhs: u16, rhs: u16): u16;

pub extern ::
u16_subtraction(lhs: u16, rhs: u16): u16;

pub extern ::
u16_right_shift(lhs: u16, rhs: u16): u16;

pub extern ::
u16_left_shift(lhs: u16, rhs: u16): u16;

pub extern ::
u16_less_than(lhs: u16, rhs: u16): bool;

pub extern ::
u16_greater_than(lhs: u16, rhs: u16): bool;

pub extern ::
u16_less_than_equals(lhs: u16, rhs: u16): bool;

pub extern ::
u16_greater_than_equals(lhs: u16, rhs: u16): bool;

pub extern ::
u16_equals(lhs: u16, rhs: u16): bool;

pub extern ::
u16_not_equals(lhs: u16, rhs: u16): bool;

pub extern ::
u16_AND(lhs: u16, rhs: u16): u16;

pub extern ::
u16_XOR(lhs: u16, rhs: u16): u16;

pub extern ::
u16_OR(lhs: u16, rhs: u16): u16;

impl Minus for u16 {
    self ::
    minus(): Self {
        ret u16_unary_negation(self);
    }
}

impl Multiply[Self] for u16 {
    self ::
    multiply(other: &Self): Self {
        ret u16_multiplication(self, other);
    }
}

impl Divide[Self] for u16 {
    self ::
    divide(other: &Self): Self {
        ret u16_division(self, other);
    }
}

impl Remainder[Self] for u16 {
    self ::
    remainder(other: &Self): Self {
        ret u16_remainder(self, other);
    }
}

impl Add[Self] for u16 {
    self ::
    add(other: &Self): Self {
        ret u16_addition(self, other);
    }
}

impl Subtract[Self] for u16 {
    self ::
    subtract(other: &Self): Self {
        ret u16_subtraction(self, other);
    }
}

impl Equal[Self] for u16 {
    self ::
    equal(other: &Self): bool {
        ret u16_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret u16_not_equals(self, other);
    }
}

impl AND[Self] for u16 {
    self ::
    and(other: &Self): Self {
        ret u16_AND(self, other);
    }
}

impl OR[Self] for u16 {
    self ::
    or(other: &Self): Self {
        ret u16_OR(self, other);
    }
}

impl XOR[Self] for u16 {
    self ::
    xor(other: &Self): Self {
        ret u16_XOR(self, other);
    }
}

pub ::
UInt32: struct {
}

pub extern ::
u32_from_i8(value: i8): u32;

pub extern ::
u32_from_i16(value: i16): u32;

pub extern ::
u32_from_i32(value: i32): u32;

pub extern ::
u32_from_i64(value: i32): u32;

pub extern ::
u32_from_u8(value: u8): u32;

pub extern ::
u32_from_u16(value: u16): u32;

pub extern ::
u32_from_u64(value: u64): u32;

pub extern ::
u32_from_int(value: int): u32;

pub extern ::
u32_from_f32(value: f32): u32;

pub extern ::
u32_from_f64(value: f64): u32;

pub extern ::
u32_unary_negation(value: u32): u32;

pub extern ::
u32_NOT(value: u32): u32;

pub extern ::
u32_multiplication(lhs: u32, rhs: u32): u32;

pub extern ::
u32_division(lhs: u32, rhs: u32): u32;

pub extern ::
u32_remainder(lhs: u32, rhs: u32): u32;

pub extern ::
u32_addition(lhs: u32, rhs: u32): u32;

pub extern ::
u32_subtraction(lhs: u32, rhs: u32): u32;

pub extern ::
u32_right_shift(lhs: u32, rhs: u32): u32;

pub extern ::
u32_left_shift(lhs: u32, rhs: u32): u32;

pub extern ::
u32_less_than(lhs: u32, rhs: u32): bool;

pub extern ::
u32_greater_than(lhs: u32, rhs: u32): bool;

pub extern ::
u32_less_than_equals(lhs: u32, rhs: u32): bool;

pub extern ::
u32_greater_than_equals(lhs: u32, rhs: u32): bool;

pub extern ::
u32_equals(lhs: u32, rhs: u32): bool;

pub extern ::
u32_not_equals(lhs: u32, rhs: u32): bool;

pub extern ::
u32_AND(lhs: u32, rhs: u32): u32;

pub extern ::
u32_XOR(lhs: u32, rhs: u32): u32;

pub extern ::
u32_OR(lhs: u32, rhs: u32): u32;

impl Minus for u32 {
    self ::
    minus(): Self {
        ret u32_unary_negation(self);
    }
}

impl Multiply[Self] for u32 {
    self ::
    multiply(other: &Self): Self {
        ret u32_multiplication(self, other);
    }
}

impl Divide[Self] for u32 {
    self ::
    divide(other: &Self): Self {
        ret u32_division(self, other);
    }
}

impl Remainder[Self] for u32 {
    self ::
    remainder(other: &Self): Self {
        ret u32_remainder(self, other);
    }
}

impl Add[Self] for u32 {
    self ::
    add(other: &Self): Self {
        ret u32_addition(self, other);
    }
}

impl Subtract[Self] for u32 {
    self ::
    subtract(other: &Self): Self {
        ret u32_subtraction(self, other);
    }
}

impl Equal[Self] for u32 {
    self ::
    equal(other: &Self): bool {
        ret u32_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret u32_not_equals(self, other);
    }
}

impl AND[Self] for u32 {
    self ::
    and(other: &Self): Self {
        ret u32_AND(self, other);
    }
}

impl OR[Self] for u32 {
    self ::
    or(other: &Self): Self {
        ret u32_OR(self, other);
    }
}

impl XOR[Self] for u32 {
    self ::
    xor(other: &Self): Self {
        ret u32_XOR(self, other);
    }
}

pub ::
UInt64: struct {
}

pub extern ::
u64_from_i8(value: i8): u64;

pub extern ::
u64_from_i16(value: i16): u64;

pub extern ::
u64_from_i32(value: i32): u64;

pub extern ::
u64_from_i64(value: i32): u64;

pub extern ::
u64_from_u8(value: u8): u64;

pub extern ::
u64_from_u16(value: u16): u64;

pub extern ::
u64_from_u32(value: u32): u64;

pub extern ::
u64_from_int(value: int): u64;

pub extern ::
u64_from_f32(value: f32): u64;

pub extern ::
u64_from_f64(value: f64): u64;

pub extern ::
u64_unary_negation(value: u64): u64;

pub extern ::
u64_NOT(value: u64): u64;

pub extern ::
u64_multiplication(lhs: u64, rhs: u64): u64;

pub extern ::
u64_division(lhs: u64, rhs: u64): u64;

pub extern ::
u64_remainder(lhs: u64, rhs: u64): u64;

pub extern ::
u64_addition(lhs: u64, rhs: u64): u64;

pub extern ::
u64_subtraction(lhs: u64, rhs: u64): u64;

pub extern ::
u64_right_shift(lhs: u64, rhs: u64): u64;

pub extern ::
u64_left_shift(lhs: u64, rhs: u64): u64;

pub extern ::
u64_less_than(lhs: u64, rhs: u64): bool;

pub extern ::
u64_greater_than(lhs: u64, rhs: u64): bool;

pub extern ::
u64_less_than_equals(lhs: u64, rhs: u64): bool;

pub extern ::
u64_greater_than_equals(lhs: u64, rhs: u64): bool;

pub extern ::
u64_equals(lhs: u64, rhs: u64): bool;

pub extern ::
u64_not_equals(lhs: u64, rhs: u64): bool;

pub extern ::
u64_AND(lhs: u64, rhs: u64): u64;

pub extern ::
u64_XOR(lhs: u64, rhs: u64): u64;

pub extern ::
u64_OR(lhs: u64, rhs: u64): u64;

impl Minus for u64 {
    self ::
    minus(): Self {
        ret u64_unary_negation(self);
    }
}

impl Multiply[Self] for u64 {
    self ::
    multiply(other: &Self): Self {
        ret u64_multiplication(self, other);
    }
}

impl Divide[Self] for u64 {
    self ::
    divide(other: &Self): Self {
        ret u64_division(self, other);
    }
}

impl Remainder[Self] for u64 {
    self ::
    remainder(other: &Self): Self {
        ret u64_remainder(self, other);
    }
}

impl Add[Self] for u64 {
    self ::
    add(other: &Self): Self {
        ret u64_addition(self, other);
    }
}

impl Subtract[Self] for u64 {
    self ::
    subtract(other: &Self): Self {
        ret u64_subtraction(self, other);
    }
}

impl Equal[Self] for u64 {
    self ::
    equal(other: &Self): bool {
        ret u64_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret u64_not_equals(self, other);
    }
}

impl AND[Self] for u64 {
    self ::
    and(other: &Self): Self {
        ret u64_AND(self, other);
    }
}

impl OR[Self] for u64 {
    self ::
    or(other: &Self): Self {
        ret u64_OR(self, other);
    }
}

impl XOR[Self] for u64 {
    self ::
    xor(other: &Self): Self {
        ret u64_XOR(self, other);
    }
}

pub ::
Int: struct {
}

pub extern ::
int_from_i8(value: i8): int;

pub extern ::
int_from_i16(value: i16): int;

pub extern ::
int_from_i32(value: i32): int;

pub extern ::
int_from_i64(value: i32): int;

pub extern ::
int_from_u8(value: u8): int;

pub extern ::
int_from_u16(value: u16): int;

pub extern ::
int_from_u32(value: u32): int;

pub extern ::
int_from_u64(value: u64): int;

pub extern ::
int_from_f32(value: f32): int;

pub extern ::
int_from_f64(value: f64): int;

pub extern ::
int_unary_negation(value: int): int;

pub extern ::
int_NOT(value: int): int;

pub extern ::
int_multiplication(lhs: int, rhs: int): int;

pub extern ::
int_division(lhs: int, rhs: int): int;

pub extern ::
int_remainder(lhs: int, rhs: int): int;

pub extern ::
int_addition(lhs: int, rhs: int): int;

pub extern ::
int_subtraction(lhs: int, rhs: int): int;

pub extern ::
int_right_shift(lhs: int, rhs: int): int;

pub extern ::
int_left_shift(lhs: int, rhs: int): int;

pub extern ::
int_less_than(lhs: int, rhs: int): bool;

pub extern ::
int_greater_than(lhs: int, rhs: int): bool;

pub extern ::
int_less_than_equals(lhs: int, rhs: int): bool;

pub extern ::
int_greater_than_equals(lhs: int, rhs: int): bool;

pub extern ::
int_equals(lhs: int, rhs: int): bool;

pub extern ::
int_not_equals(lhs: int, rhs: int): bool;

pub extern ::
int_AND(lhs: int, rhs: int): int;

pub extern ::
int_XOR(lhs: int, rhs: int): int;

pub extern ::
int_OR(lhs: int, rhs: int): int;

impl Minus for int {
    self ::
    minus(): Self {
        ret int_unary_negation(self);
    }
}

impl Multiply[Self] for int {
    self ::
    multiply(other: &Self): Self {
        ret int_multiplication(self, other);
    }
}

impl Divide[Self] for int {
    self ::
    divide(other: &Self): Self {
        ret int_division(self, other);
    }
}

impl Remainder[Self] for int {
    self ::
    remainder(other: &Self): Self {
        ret int_remainder(self, other);
    }
}

impl Add[Self] for int {
    self ::
    add(other: &Self): Self {
        ret int_addition(self, other);
    }
}

impl Subtract[Self] for int {
    self ::
    subtract(other: &Self): Self {
        ret int_subtraction(self, other);
    }
}

impl Equal[Self] for int {
    self ::
    equal(other: &Self): bool {
        ret int_equals(self, other);
    }

    self ::
    not_equal(other: &Self): bool {
        ret int_not_equals(self, other);
    }
}

impl AND[Self] for int {
    self ::
    and(other: &Self): Self {
        ret int_AND(self, other);
    }
}

impl OR[Self] for int {
    self ::
    or(other: &Self): Self {
        ret int_OR(self, other);
    }
}

impl XOR[Self] for int {
    self ::
    xor(other: &Self): Self {
        ret int_XOR(self, other);
    }
}
)";

static inline constexpr const char* mem = R"(
pub ::
mem: mod {
    pub extern ::
    alloc(block_size: int): Ptr;

    pub extern ::
    dealloc(block: Ptr): void;

    pub extern ::
    copy(src_block: Ptr, dst_block: Ptr, block_size: int): void;
}
)";

static inline constexpr const char* op = R"(
pub ::
Minus: trait {
    self ::
    minus(): Self;
}

pub ::
Multiply[Other]: trait {
    self ::
    multiply(other: &Other): Self;
}

pub ::
Divide[Other]: trait {
    self ::
    divide(other: &Other): Self;
}

pub ::
Remainder[Other]: trait {
    self ::
    remainder(other: &Other): Self;
}

pub ::
Add[Other]: trait {
    self ::
    add(other: &Other): Self;
}

pub ::
Subtract[Other]: trait {
    self ::
    subtract(other: &Other): Self;
}

pub ::
Equal[Other]: trait {
    self ::
    equal(other: &Other): bool;

    self ::
    not_equal(other: &Other): bool;
}

pub ::
AND[Other]: trait {
    self ::
    and(other: &Other): Self;
}

pub ::
OR[Other]: trait {
    self ::
    or(other: &Other): Self;
}

pub ::
XOR[Other]: trait {
    self ::
    xor(other: &Other): Self;
}

pub ::
Lifetime: trait {
    self ::
    copy(other: &Self): void;

    self ::
    drop(): void;
}
)";

static inline constexpr const char* print = R"(
pub extern ::
print_int(value: int): void;

pub extern ::
print_ptr(ptr: Ptr): void;
)";

static inline constexpr const char* ptr = R"(
pub ::
Ptr: struct {
}
)";

static inline constexpr const char* rc = R"(
pub ::
rc: mod {
    ControlBlock: struct {
        value_ptr:     Ptr,
        type_info_ptr: Ptr,
        strong_count:  int,
        weak_count:    int,
    }

    impl ControlBlock {
        value_ptr(self_ptr: Ptr): Ptr {
            ret __deref_as[Self](self_ptr).value_ptr;
        }

        type_info_ptr(self_ptr: Ptr): Ptr {
            ret __deref_as[Self](self_ptr).type_info_ptr;
        }

        strong_count(self_ptr: Ptr): int {
            ret __deref_as[Self](self_ptr).strong_count;
        }

        set_strong_count(self_ptr: Ptr, value: int): void {
            __deref_as[Self](self_ptr).strong_count = value;
        }

        increment_strong_count(self_ptr: Ptr): void {
            set_strong_count(self_ptr, strong_count(self_ptr) + 1);
        }

        decrement_strong_count(self_ptr: Ptr): void {
            assert strong_count(self_ptr) > 0;
            set_strong_count(self_ptr, strong_count(self_ptr) - 1);
        }

        weak_count(self_ptr: Ptr): int {
            ret __deref_as[Self](self_ptr).weak_count;
        }

        set_weak_count(self_ptr: Ptr, value: int): void {
            __deref_as[Self](self_ptr).weak_count = value;
        }

        increment_weak_count(self_ptr: Ptr): void {
            set_weak_count(self_ptr, weak_count(self_ptr) + 1);
        }

        decrement_weak_count(self_ptr: Ptr): void {
            assert weak_count(self_ptr) > 0;
            set_weak_count(self_ptr, weak_count(self_ptr) - 1);
        }
    }

    pub ::
    WeakPtr[T]: struct {
        control_block_ptr: Ptr,
    }

    pub ::
    weak_ptr_from[T](strong_ptr: &StrongPtr[T]): WeakPtr[T] {
        control_block_ptr: Ptr = strong_ptr_control_block_ptr[T](strong_ptr);
        ControlBlock::increment_weak_count(control_block_ptr);
        ret new WeakPtr[T] { control_block_ptr };
    }

    pub ::
    weak_ptr_from_dyn[T](dyn_strong_ptr: &DynStrongPtr[T]): WeakPtr[T] {
        control_block_ptr: Ptr =
            dyn_strong_ptr_control_block_ptr[T](dyn_strong_ptr);
        ControlBlock::increment_weak_count(control_block_ptr);
        ret new WeakPtr[T] { control_block_ptr };
    }

    pub ::
    weak_ptr_copy[T](this: &WeakPtr[T], other: &WeakPtr[T]): void {
        this.control_block_ptr = other.control_block_ptr;

        ControlBlock::increment_weak_count(this.control_block_ptr);
    }

    pub ::
    weak_ptr_drop[T](this: &WeakPtr[T]): void {
        ControlBlock::decrement_weak_count(this.control_block_ptr);

        if 
            (ControlBlock::strong_count(this.control_block_ptr) == 0) && 
            (ControlBlock::weak_count(this.control_block_ptr) == 0) 
        {
            mem::dealloc(this.control_block_ptr);
        }
    }

    pub ::
    weak_ptr_lock[T](this: &WeakPtr[T]): StrongPtr[T] {
        assert ControlBlock::strong_count(this.control_block_ptr) > 0;
        ret strong_ptr_from[T](this);
    }

    pub ::
    weak_ptr_lock_dyn[T](this: &WeakPtr[T]): DynStrongPtr[T] {
        assert ControlBlock::strong_count(this.control_block_ptr) > 0;
        ret dyn_strong_ptr_from[T](this);
    }

    pub ::
    StrongPtr[T]: struct {
        data: StrongPtrData
    }

    StrongPtrData: struct {
        value_ptr:         Ptr,
        control_block_ptr: Ptr,
    }

    pub ::
    strong_ptr_new[T](value: &T): StrongPtr[T] {
        value_ptr:         Ptr = mem::alloc(__size_of[T]);
        control_block_ptr: Ptr = mem::alloc(__size_of[ControlBlock]);

        __copy[T](Ref[T]::ptr(value), value_ptr);

        type_info_ptr: Ptr = __type_info_ptr[T];
        control_block: ControlBlock = new ControlBlock {
            value_ptr,
            type_info_ptr,
            strong_count: 1, 
            weak_count:   0,
        };
        mem::copy(
            __address_of(control_block),
            control_block_ptr,
            __size_of[ControlBlock]
        );

        data: StrongPtrData = new StrongPtrData {
            value_ptr,
            control_block_ptr,
        };
        ret new StrongPtr[T] { data };
    }

    strong_ptr_from[T](weak_ptr: &WeakPtr[T]): StrongPtr[T] {
        control_block_ptr: Ptr = weak_ptr.control_block_ptr;
        value_ptr:         Ptr = ControlBlock::value_ptr(control_block_ptr);

        ControlBlock::increment_strong_count(control_block_ptr);

        data: StrongPtrData = new StrongPtrData {
            value_ptr,
            control_block_ptr,
        };
        ret new StrongPtr[T] { data };
    }

    pub ::
    strong_ptr_copy[T](this: &StrongPtr[T], other: &StrongPtr[T]): void {
        value_ptr:         Ptr = strong_ptr_value_ptr        [T](other);
        control_block_ptr: Ptr = strong_ptr_control_block_ptr[T](other);

        strong_ptr_set_value_ptr        [T](this, value_ptr);
        strong_ptr_set_control_block_ptr[T](this, control_block_ptr);

        ControlBlock::increment_strong_count(control_block_ptr);
    }

    pub ::
    strong_ptr_drop[T](this: &StrongPtr[T]): void {
        value_ptr:         Ptr = strong_ptr_value_ptr        [T](this);
        control_block_ptr: Ptr = strong_ptr_control_block_ptr[T](this);

        ControlBlock::decrement_strong_count(control_block_ptr);

        if ControlBlock::strong_count(control_block_ptr) == 0 {
            __drop[T](value_ptr);
            mem::dealloc(value_ptr);

            if ControlBlock::weak_count(control_block_ptr) == 0 {
                mem::dealloc(control_block_ptr);
            }
        }
    }

    strong_ptr_value_ptr[T](this: &StrongPtr[T]): Ptr {
        ret __deref_as[StrongPtrData](this).value_ptr;
    }

    strong_ptr_set_value_ptr[T](this: &StrongPtr[T], value: Ptr): void {
        __deref_as[StrongPtrData](this).value_ptr = value;
    }

    strong_ptr_control_block_ptr[T](this: &StrongPtr[T]): Ptr {
        ret __deref_as[StrongPtrData](this).control_block_ptr;
    }

    strong_ptr_set_control_block_ptr[T](this: &StrongPtr[T], value: Ptr): void {
        __deref_as[StrongPtrData](this).control_block_ptr = value;
    }

    pub ::
    strong_ptr_value[T](this: &StrongPtr[T]): T {
        ret __deref_as[T](strong_ptr_value_ptr[T](this));
    }

    pub ::
    DynStrongPtrData: struct {
        value_ptr:         Ptr,
        control_block_ptr: Ptr,
        vtbl_ptr:          Ptr,
    }

    pub ::
    DynStrongPtr[T]: struct {
        data: DynStrongPtrData
    }

    pub ::
    dyn_strong_ptr_from[T](weak_ptr: &WeakPtr[T]): DynStrongPtr[T] {
        control_block_ptr: Ptr = weak_ptr.control_block_ptr;
        value_ptr:         Ptr = ControlBlock::value_ptr(control_block_ptr);
        type_info_ptr:     Ptr = ControlBlock::type_info_ptr(control_block_ptr);

        vtbl_ptr: Ptr = lookup_vtbl_ptr(type_info_ptr, __type_info_ptr[T]);

        ControlBlock::increment_strong_count(control_block_ptr);

        data: DynStrongPtrData = new DynStrongPtrData {
            value_ptr,
            control_block_ptr,
            vtbl_ptr,
        };
        ret new DynStrongPtr[T] { data };
    }

    pub ::
    dyn_strong_ptr_copy[T](
        this:  &DynStrongPtr[T],
        other: &DynStrongPtr[T]
    ): void
    {
        value_ptr:         Ptr = dyn_strong_ptr_value_ptr        [T](other);
        vtbl_ptr:          Ptr = dyn_strong_ptr_vtbl_ptr         [T](other);
        control_block_ptr: Ptr = dyn_strong_ptr_control_block_ptr[T](other);

        dyn_strong_ptr_set_value_ptr        [T](this, value_ptr);
        dyn_strong_ptr_set_vtbl_ptr         [T](this, vtbl_ptr);
        dyn_strong_ptr_set_control_block_ptr[T](this, control_block_ptr);

        ControlBlock::increment_strong_count(control_block_ptr);
    }

    pub ::
    dyn_strong_ptr_drop[T](this: &DynStrongPtr[T]): void {
        value_ptr:         Ptr = dyn_strong_ptr_value_ptr        [T](this);
        vtbl_ptr:          Ptr = dyn_strong_ptr_vtbl_ptr         [T](this);
        control_block_ptr: Ptr = dyn_strong_ptr_control_block_ptr[T](this);

        ControlBlock::decrement_strong_count(control_block_ptr);

        if ControlBlock::strong_count(control_block_ptr) == 0 {
            dyn_drop(value_ptr, ControlBlock::type_info_ptr(control_block_ptr));
            mem::dealloc(value_ptr);

            if ControlBlock::weak_count(control_block_ptr) == 0 {
                mem::dealloc(control_block_ptr);
            }
        }
    }

    pub ::
    dyn_strong_ptr_value_ptr[T](this: &DynStrongPtr[T]): Ptr {
        ret __deref_as[DynStrongPtrData](this).value_ptr;
    }

    pub ::
    dyn_strong_ptr_set_value_ptr[T](this: &DynStrongPtr[T], value: Ptr): void {
        __deref_as[DynStrongPtrData](this).value_ptr = value;
    }

    pub ::
    dyn_strong_ptr_control_block_ptr[T](this: &DynStrongPtr[T]): Ptr {
        ret __deref_as[DynStrongPtrData](this).control_block_ptr;
    }

    pub ::
    dyn_strong_ptr_set_control_block_ptr[T](
        this: &DynStrongPtr[T],
        value: Ptr
    ): void
    {
        __deref_as[DynStrongPtrData](this).control_block_ptr = value;
    }

    pub ::
    dyn_strong_ptr_vtbl_ptr[T](this: &DynStrongPtr[T]): Ptr {
        ret __deref_as[DynStrongPtrData](this).vtbl_ptr;
    }

    pub ::
    dyn_strong_ptr_set_vtbl_ptr[T](this: &DynStrongPtr[T], value: Ptr): void {
        __deref_as[DynStrongPtrData](this).vtbl_ptr = value;
    }

    pub extern ::
    lookup_vtbl_ptr(type_info_ptr: Ptr, trait_type_info_ptr: Ptr): Ptr;

    pub extern ::
    sublookup_vtbl_ptr(type_info_ptr: Ptr, target_type_info_ptr: Ptr): Ptr;

    pub extern ::
    dyn_drop(value_ptr: Ptr, type_info_ptr: Ptr): void;

    pub ::
    strong_ptr_to_dyn_strong_ptr[From, To](
        from: &StrongPtr[From]
    ): DynStrongPtr[To]
    {
        value_ptr:         Ptr = strong_ptr_value_ptr[From](from);
        vtbl_ptr:          Ptr = __vtbl_ptr[From, To];
        control_block_ptr: Ptr = strong_ptr_control_block_ptr[From](from);

        ControlBlock::increment_strong_count(control_block_ptr);
        
        data: DynStrongPtrData = new DynStrongPtrData {
            value_ptr,
            vtbl_ptr,
            control_block_ptr
        };
        ret new DynStrongPtr[To] { data };
    }
}
)";

static inline constexpr const char* ref = R"(
pub ::
Ref[T]: struct {
    ptr: Ptr
}

impl[T] Ref[T] {
    ptr(this: Self): Ptr {
        ret __deref_as[Ptr](__address_of(this));
    }
}
)";

static inline constexpr const char* string = R"(
pub ::
String: struct {
}
)";

namespace Ace::Std
{
    auto GetName() -> const std::string&
    {
        static const std::string name{ "std" };
        return name;
    }

    auto CreateFileBuffers(
        Compilation* const compilation
    ) -> std::vector<std::shared_ptr<const FileBuffer>>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        std::vector<std::string_view> buffers
        {
            bool_,
            float_,
            int_,
            mem,
            op,
            print,
            ptr,
            rc,
            ref,
            string,
        };

        std::vector<std::shared_ptr<const FileBuffer>> fileBuffers{};
        std::transform(begin(buffers), end(buffers), back_inserter(fileBuffers),
        [&](const std::string_view buffer)
        {
            return FileBuffer::Create(compilation, buffer);
        });

        return fileBuffers;
    }   
}
