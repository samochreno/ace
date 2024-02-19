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
f32_unary_plus(value: f32): f32;

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
f64_unary_plus(value: f64): f64;

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
i8_unary_plus(value: i8): i8;

pub extern ::
i8_unary_negation(value: i8): i8;

pub extern ::
i8_one_complement(value: i8): i8;

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
i16_unary_plus(value: i16): i16;

pub extern ::
i16_unary_negation(value: i16): i16;

pub extern ::
i16_one_complement(value: i16): i16;

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
i32_unary_plus(value: i32): i32;

pub extern ::
i32_unary_negation(value: i32): i32;

pub extern ::
i32_one_complement(value: i32): i32;

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
i64_unary_plus(value: i64): i64;

pub extern ::
i64_unary_negation(value: i64): i64;

pub extern ::
i64_one_complement(value: i64): i64;

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
u8_unary_plus(value: u8): u8;

pub extern ::
u8_unary_negation(value: u8): u8;

pub extern ::
u8_one_complement(value: u8): u8;

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
u16_unary_plus(value: u16): u16;

pub extern ::
u16_unary_negation(value: u16): u16;

pub extern ::
u16_one_complement(value: u16): u16;

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
u32_unary_plus(value: u32): u32;

pub extern ::
u32_unary_negation(value: u32): u32;

pub extern ::
u32_one_complement(value: u32): u32;

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
u64_unary_plus(value: u64): u64;

pub extern ::
u64_unary_negation(value: u64): u64;

pub extern ::
u64_one_complement(value: u64): u64;

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
int_unary_plus(value: int): int;

pub extern ::
int_unary_negation(value: int): int;

pub extern ::
int_one_complement(value: int): int;

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
