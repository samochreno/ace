#pragma once

#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>
#include <set>

#include "LLVM.hpp"
#include "Diagnostic.hpp"
#include "Scope.hpp"
#include "Lazy.hpp"
#include "Op.hpp"

namespace llvm
{
    class LLVMContext;
}

namespace Ace
{
    class ITypeSymbol;
    class FunctionSymbol;

    class Emitter;

    using FunctionBlockEmitter = std::function<void(Emitter&)>;

    class INative
    {
    public:
        virtual ~INative() = default;

        virtual auto GetCompilation() const -> Compilation* = 0;

        virtual auto CreateFullyQualifiedName(
            const SrcLocation& srcLocation
        ) const -> SymbolName = 0;

        virtual auto GetGenericSymbol() const -> ISymbol* = 0;
    };

    enum class NativeSymbolKind
    {
        Concrete,
        Root,
    };

    enum class NativeCopyabilityKind
    {
        Trivial,
        NonTrivial,
    };

    class NativeType : public virtual INative
    {
    public:
        NativeType(
            Compilation* const compilation,
            std::vector<const char*> nameSectionStrings,
            const NativeSymbolKind symbolKind,
            const NativeCopyabilityKind copyabilityKind,
            std::optional<std::function<llvm::Type*(llvm::LLVMContext&)>> irTypeGetter
        );
        ~NativeType() = default;

        auto GetCompilation() const -> Compilation* final;

        auto CreateFullyQualifiedName(
            const SrcLocation& srcLocation
        ) const -> SymbolName final;

        auto TryGetSymbol() const -> std::optional<ITypeSymbol*>;
        auto GetSymbol() const -> ITypeSymbol*;
        auto GetGenericSymbol() const -> ISymbol* final;

        auto HasIRType() const -> bool;
        auto GetIRType(llvm::LLVMContext& context) const -> llvm::Type*;
        
    private:
        Compilation* m_Compilation{};
        std::vector<const char*> m_NameSectionStrings{};
        std::optional<std::function<llvm::Type*(llvm::LLVMContext&)>> m_IRTypeGetter{};
        bool m_IsRoot{};
        bool m_IsTriviallyCopyable{};
        
        mutable std::optional<ITypeSymbol*> m_OptSymbol{};
    };

    class NativeFunction : public virtual INative
    {
    public:
        NativeFunction(
            Compilation* const compilation,
            std::vector<std::string> nameSectionStrings,
            const NativeSymbolKind kind,
            std::optional<FunctionBlockEmitter> optBlockEmitter
        );
        ~NativeFunction() = default;

        auto GetCompilation() const -> Compilation* final;

        auto CreateFullyQualifiedName(
            const SrcLocation& srcLocation
        ) const -> SymbolName final;

        auto TryGetSymbol() const -> std::optional<FunctionSymbol*>;
        auto GetSymbol() const -> FunctionSymbol*;
        auto GetGenericSymbol() const -> ISymbol* final;
        
    private:
        Compilation* m_Compilation{};
        std::vector<std::string> m_NameSectionStrings{};
        bool m_IsRoot{};
        std::optional<FunctionBlockEmitter> m_OptBlockEmitter{};

        mutable std::optional<FunctionSymbol*> m_OptSymbol{};
    };

    class Natives
    {
    public:
        Natives(Compilation* const compilation);
        ~Natives() = default;

        auto Verify() const -> void;

        auto CollectIRTypeSymbolMap(
            llvm::LLVMContext& context
        ) const -> std::unordered_map<ITypeSymbol*, llvm::Type*>;

        auto GetImplicitFromOpMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>&;
        auto GetExplicitFromOpMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>&;

        auto GetUnaryOpMap()  const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>>&;
        auto GetBinaryOpMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>>&;

        auto GetCopyOpMap() const -> const std::unordered_map<ITypeSymbol*, FunctionSymbol*>&;
        auto GetDropOpMap() const -> const std::unordered_map<ITypeSymbol*, FunctionSymbol*>&;

        auto IsIntTypeSigned(const NativeType& intType) const -> bool;

        NativeType Int8;
        NativeType Int16;
        NativeType Int32;
        NativeType Int64;
    
        NativeType UInt8;
        NativeType UInt16;
        NativeType UInt32;
        NativeType UInt64;

        NativeType Int;
    
        NativeType Float32;
        NativeType Float64;

        NativeType Bool;
        NativeType String;

        NativeType Ptr;

        NativeType Ref;
        NativeType WeakPtr;
        NativeType StrongPtr;
        NativeType DynStrongPtr;
        NativeType DynStrongPtrData;

        NativeType Minus;
        NativeType Multiply;
        NativeType Divide;
        NativeType Remainder;
        NativeType Add;
        NativeType Subtract;
        NativeType Equal;
        NativeType AND;
        NativeType OR;
        NativeType XOR;
        NativeType Lifetime;

        NativeFunction print_int;
        NativeFunction print_ptr;

        NativeFunction alloc;
        NativeFunction dealloc;
        NativeFunction copy;

        NativeFunction lookup_vtbl_ptr;
        NativeFunction sublookup_vtbl_ptr;
        NativeFunction dyn_drop;

        NativeFunction i8_from_i16;
        NativeFunction i8_from_i32;
        NativeFunction i8_from_i64;
        NativeFunction i8_from_u8;
        NativeFunction i8_from_u16;
        NativeFunction i8_from_u32;
        NativeFunction i8_from_u64;
        NativeFunction i8_from_int;
        NativeFunction i8_from_f32;
        NativeFunction i8_from_f64;
        NativeFunction i8_unary_negation;
        NativeFunction i8_NOT;
        NativeFunction i8_multiplication;
        NativeFunction i8_division;
        NativeFunction i8_remainder;
        NativeFunction i8_addition;
        NativeFunction i8_subtraction;
        NativeFunction i8_right_shift;
        NativeFunction i8_left_shift;
        NativeFunction i8_less_than;
        NativeFunction i8_greater_than;
        NativeFunction i8_less_than_equals;
        NativeFunction i8_greater_than_equals;
        NativeFunction i8_equals;
        NativeFunction i8_not_equals;
        NativeFunction i8_AND;
        NativeFunction i8_XOR;
        NativeFunction i8_OR;

        NativeFunction i16_from_i8;
        NativeFunction i16_from_i32;
        NativeFunction i16_from_i64;
        NativeFunction i16_from_u8;
        NativeFunction i16_from_u16;
        NativeFunction i16_from_u32;
        NativeFunction i16_from_u64;
        NativeFunction i16_from_int;
        NativeFunction i16_from_f32;
        NativeFunction i16_from_f64;
        NativeFunction i16_unary_negation;
        NativeFunction i16_NOT;
        NativeFunction i16_multiplication;
        NativeFunction i16_division;
        NativeFunction i16_remainder;
        NativeFunction i16_addition;
        NativeFunction i16_subtraction;
        NativeFunction i16_right_shift;
        NativeFunction i16_left_shift;
        NativeFunction i16_less_than;
        NativeFunction i16_greater_than;
        NativeFunction i16_less_than_equals;
        NativeFunction i16_greater_than_equals;
        NativeFunction i16_equals;
        NativeFunction i16_not_equals;
        NativeFunction i16_AND;
        NativeFunction i16_XOR;
        NativeFunction i16_OR;

        NativeFunction i32_from_i8;
        NativeFunction i32_from_i16;
        NativeFunction i32_from_i64;
        NativeFunction i32_from_u8;
        NativeFunction i32_from_u16;
        NativeFunction i32_from_u32;
        NativeFunction i32_from_u64;
        NativeFunction i32_from_int;
        NativeFunction i32_from_f32;
        NativeFunction i32_from_f64;
        NativeFunction i32_unary_negation;
        NativeFunction i32_NOT;
        NativeFunction i32_multiplication;
        NativeFunction i32_division;
        NativeFunction i32_remainder;
        NativeFunction i32_addition;
        NativeFunction i32_subtraction;
        NativeFunction i32_right_shift;
        NativeFunction i32_left_shift;
        NativeFunction i32_less_than;
        NativeFunction i32_greater_than;
        NativeFunction i32_less_than_equals;
        NativeFunction i32_greater_than_equals;
        NativeFunction i32_equals;
        NativeFunction i32_not_equals;
        NativeFunction i32_AND;
        NativeFunction i32_XOR;
        NativeFunction i32_OR;

        NativeFunction i64_from_i8;
        NativeFunction i64_from_i16;
        NativeFunction i64_from_i32;
        NativeFunction i64_from_u8;
        NativeFunction i64_from_u16;
        NativeFunction i64_from_u32;
        NativeFunction i64_from_u64;
        NativeFunction i64_from_int;
        NativeFunction i64_from_f32;
        NativeFunction i64_from_f64;
        NativeFunction i64_unary_negation;
        NativeFunction i64_NOT;
        NativeFunction i64_multiplication;
        NativeFunction i64_division;
        NativeFunction i64_remainder;
        NativeFunction i64_addition;
        NativeFunction i64_subtraction;
        NativeFunction i64_right_shift;
        NativeFunction i64_left_shift;
        NativeFunction i64_less_than;
        NativeFunction i64_greater_than;
        NativeFunction i64_less_than_equals;
        NativeFunction i64_greater_than_equals;
        NativeFunction i64_equals;
        NativeFunction i64_not_equals;
        NativeFunction i64_AND;
        NativeFunction i64_XOR;
        NativeFunction i64_OR;

        NativeFunction u8_from_i8;
        NativeFunction u8_from_i16;
        NativeFunction u8_from_i32;
        NativeFunction u8_from_i64;
        NativeFunction u8_from_u16;
        NativeFunction u8_from_u32;
        NativeFunction u8_from_u64;
        NativeFunction u8_from_int;
        NativeFunction u8_from_f32;
        NativeFunction u8_from_f64;
        NativeFunction u8_unary_negation;
        NativeFunction u8_NOT;
        NativeFunction u8_multiplication;
        NativeFunction u8_division;
        NativeFunction u8_remainder;
        NativeFunction u8_addition;
        NativeFunction u8_subtraction;
        NativeFunction u8_right_shift;
        NativeFunction u8_left_shift;
        NativeFunction u8_less_than;
        NativeFunction u8_greater_than;
        NativeFunction u8_less_than_equals;
        NativeFunction u8_greater_than_equals;
        NativeFunction u8_equals;
        NativeFunction u8_not_equals;
        NativeFunction u8_AND;
        NativeFunction u8_XOR;
        NativeFunction u8_OR;

        NativeFunction u16_from_i8;
        NativeFunction u16_from_i16;
        NativeFunction u16_from_i32;
        NativeFunction u16_from_i64;
        NativeFunction u16_from_u8;
        NativeFunction u16_from_u32;
        NativeFunction u16_from_u64;
        NativeFunction u16_from_int;
        NativeFunction u16_from_f32;
        NativeFunction u16_from_f64;
        NativeFunction u16_unary_negation;
        NativeFunction u16_NOT;
        NativeFunction u16_multiplication;
        NativeFunction u16_division;
        NativeFunction u16_remainder;
        NativeFunction u16_addition;
        NativeFunction u16_subtraction;
        NativeFunction u16_right_shift;
        NativeFunction u16_left_shift;
        NativeFunction u16_less_than;
        NativeFunction u16_greater_than;
        NativeFunction u16_less_than_equals;
        NativeFunction u16_greater_than_equals;
        NativeFunction u16_equals;
        NativeFunction u16_not_equals;
        NativeFunction u16_AND;
        NativeFunction u16_XOR;
        NativeFunction u16_OR;

        NativeFunction u32_from_i8;
        NativeFunction u32_from_i16;
        NativeFunction u32_from_i32;
        NativeFunction u32_from_i64;
        NativeFunction u32_from_u8;
        NativeFunction u32_from_u16;
        NativeFunction u32_from_u64;
        NativeFunction u32_from_int;
        NativeFunction u32_from_f32;
        NativeFunction u32_from_f64;
        NativeFunction u32_unary_negation;
        NativeFunction u32_NOT;
        NativeFunction u32_multiplication;
        NativeFunction u32_division;
        NativeFunction u32_remainder;
        NativeFunction u32_addition;
        NativeFunction u32_subtraction;
        NativeFunction u32_right_shift;
        NativeFunction u32_left_shift;
        NativeFunction u32_less_than;
        NativeFunction u32_greater_than;
        NativeFunction u32_less_than_equals;
        NativeFunction u32_greater_than_equals;
        NativeFunction u32_equals;
        NativeFunction u32_not_equals;
        NativeFunction u32_AND;
        NativeFunction u32_XOR;
        NativeFunction u32_OR;

        NativeFunction u64_from_i8;
        NativeFunction u64_from_i16;
        NativeFunction u64_from_i32;
        NativeFunction u64_from_i64;
        NativeFunction u64_from_u8;
        NativeFunction u64_from_u16;
        NativeFunction u64_from_u32;
        NativeFunction u64_from_int;
        NativeFunction u64_from_f32;
        NativeFunction u64_from_f64;
        NativeFunction u64_unary_negation;
        NativeFunction u64_NOT;
        NativeFunction u64_multiplication;
        NativeFunction u64_division;
        NativeFunction u64_remainder;
        NativeFunction u64_addition;
        NativeFunction u64_subtraction;
        NativeFunction u64_right_shift;
        NativeFunction u64_left_shift;
        NativeFunction u64_less_than;
        NativeFunction u64_greater_than;
        NativeFunction u64_less_than_equals;
        NativeFunction u64_greater_than_equals;
        NativeFunction u64_equals;
        NativeFunction u64_not_equals;
        NativeFunction u64_AND;
        NativeFunction u64_XOR;
        NativeFunction u64_OR;

        NativeFunction int_from_i8;
        NativeFunction int_from_i16;
        NativeFunction int_from_i32;
        NativeFunction int_from_i64;
        NativeFunction int_from_u8;
        NativeFunction int_from_u16;
        NativeFunction int_from_u32;
        NativeFunction int_from_u64;
        NativeFunction int_from_f32;
        NativeFunction int_from_f64;
        NativeFunction int_unary_negation;
        NativeFunction int_NOT;
        NativeFunction int_multiplication;
        NativeFunction int_division;
        NativeFunction int_remainder;
        NativeFunction int_addition;
        NativeFunction int_subtraction;
        NativeFunction int_right_shift;
        NativeFunction int_left_shift;
        NativeFunction int_less_than;
        NativeFunction int_greater_than;
        NativeFunction int_less_than_equals;
        NativeFunction int_greater_than_equals;
        NativeFunction int_equals;
        NativeFunction int_not_equals;
        NativeFunction int_AND;
        NativeFunction int_XOR;
        NativeFunction int_OR;

        NativeFunction f32_from_i8;
        NativeFunction f32_from_i16;
        NativeFunction f32_from_i32;
        NativeFunction f32_from_i64;
        NativeFunction f32_from_u8;
        NativeFunction f32_from_u16;
        NativeFunction f32_from_u32;
        NativeFunction f32_from_u64;
        NativeFunction f32_from_int;
        NativeFunction f32_from_f64;
        NativeFunction f32_unary_negation;
        NativeFunction f32_multiplication;
        NativeFunction f32_division;
        NativeFunction f32_remainder;
        NativeFunction f32_addition;
        NativeFunction f32_subtraction;
        NativeFunction f32_less_than;
        NativeFunction f32_greater_than;
        NativeFunction f32_less_than_equals;
        NativeFunction f32_greater_than_equals;
        NativeFunction f32_equals;
        NativeFunction f32_not_equals;

        NativeFunction f64_from_i8;
        NativeFunction f64_from_i16;
        NativeFunction f64_from_i32;
        NativeFunction f64_from_i64;
        NativeFunction f64_from_u8;
        NativeFunction f64_from_u16;
        NativeFunction f64_from_u32;
        NativeFunction f64_from_u64;
        NativeFunction f64_from_int;
        NativeFunction f64_from_f32;
        NativeFunction f64_unary_negation;
        NativeFunction f64_multiplication;
        NativeFunction f64_division;
        NativeFunction f64_remainder;
        NativeFunction f64_addition;
        NativeFunction f64_subtraction;
        NativeFunction f64_less_than;
        NativeFunction f64_greater_than;
        NativeFunction f64_less_than_equals;
        NativeFunction f64_greater_than_equals;
        NativeFunction f64_equals;
        NativeFunction f64_not_equals;

        NativeFunction weak_ptr_from;
        NativeFunction weak_ptr_from_dyn;
        NativeFunction weak_ptr_copy;
        NativeFunction weak_ptr_drop;
        NativeFunction weak_ptr_lock;
        NativeFunction weak_ptr_lock_dyn;

        NativeFunction strong_ptr_new;
        NativeFunction strong_ptr_value;
        NativeFunction strong_ptr_copy;
        NativeFunction strong_ptr_drop;

        NativeFunction dyn_strong_ptr_from;
        NativeFunction dyn_strong_ptr_copy;
        NativeFunction dyn_strong_ptr_drop;
        NativeFunction dyn_strong_ptr_value_ptr;
        NativeFunction dyn_strong_ptr_set_value_ptr;
        NativeFunction dyn_strong_ptr_vtbl_ptr;
        NativeFunction dyn_strong_ptr_set_vtbl_ptr;
        NativeFunction dyn_strong_ptr_control_block_ptr;
        NativeFunction dyn_strong_ptr_set_control_block_ptr;

        NativeFunction strong_ptr_to_dyn_strong_ptr;

    private:
        Lazy<std::vector<INative*>> m_Natives;
        Lazy<std::vector<NativeType*>> m_Types;
        Lazy<std::vector<NativeFunction*>> m_Functions;

        Lazy<std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>> m_ImplicitFromOpMap;
        Lazy<std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>> m_ExplicitFromOpMap;

        Lazy<std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>>> m_UnaryOpMap;
        Lazy<std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>>> m_BinaryOpMap;

        Lazy<std::unordered_map<ITypeSymbol*, FunctionSymbol*>> m_CopyOpMap;
        Lazy<std::unordered_map<ITypeSymbol*, FunctionSymbol*>> m_DropOpMap;

        Lazy<std::set<const NativeType*>> m_SignedIntTypesSet;
    };
}
