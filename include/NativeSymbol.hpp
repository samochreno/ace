#pragma once

#include <vector>
#include <optional>
#include <functional>

#include "LLVM.hpp"
#include "Error.hpp"
#include "Token.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace
{
    class Emitter;
}

namespace Ace::Symbol
{
    class Function;

    namespace Type
    {
        class IBase;
    }

    namespace Template
    {
        class Type;
        class Function;
    }
}

namespace Ace::NativeSymbol
{
    using FunctionBodyEmitter = std::function<void(Emitter&)>;

    class IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto Initialize() -> Expected<void> = 0;
    };

    class ITypeable : public virtual IBase
    {
    public:
        virtual ~ITypeable() = default;

        virtual auto GetFullyQualifiedName() const -> const Name::Symbol::Full& = 0;
    };

    class Type : public virtual ITypeable
    {
    public:
        Type(
            Name::Symbol::Full&& t_name,
            std::optional<std::function<llvm::Type*(Emitter&)>>&& t_irTypeGetter
        ) : m_Name{ std::move(t_name) },
            m_IRTypeGetter{ std::move(t_irTypeGetter) }
        {
        }
        virtual ~Type() = default;

        auto GetFullyQualifiedName() const -> const Name::Symbol::Full& final { return m_Name; }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Type::IBase* { return m_Symbol; }

        auto HasIRType() const -> bool { return m_IRTypeGetter.has_value(); }
        auto GetIRType(Emitter& t_emitter) const -> llvm::Type*
        {
            ACE_ASSERT(HasIRType());
            return m_IRTypeGetter.value()(t_emitter);
        }

    private:
        Symbol::Type::IBase* m_Symbol{};
        Name::Symbol::Full m_Name{};
        std::optional<std::function<llvm::Type*(Emitter&)>> m_IRTypeGetter{};
    };

    class TypeTemplate : public virtual ITypeable
    {
    public:
        TypeTemplate(Name::Symbol::Full&& t_name)
            : m_Name{ std::move(t_name) }
        {
        }
        virtual ~TypeTemplate() = default;

        auto GetFullyQualifiedName() const -> const Name::Symbol::Full& final { return m_Name; }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Template::Type* { return m_Symbol; }

    private:
        Symbol::Template::Type* m_Symbol{};
        Name::Symbol::Full m_Name{};
    };
    
    class Function : public virtual IBase
    {
    public:
        Function(
            Name::Symbol::Full&& t_name,
            FunctionBodyEmitter&& t_bodyEmitter
        ) : m_Name{ std::move(t_name) },
            m_BodyEmitter{ std::move(t_bodyEmitter) }
        {
        }
        virtual ~Function() = default;

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Function* { return m_Symbol; }

    private:
        Symbol::Function* m_Symbol{};
        Name::Symbol::Full m_Name{};
        FunctionBodyEmitter m_BodyEmitter{};
    };

    class FunctionTemplate : public virtual IBase
    {
    public:
        FunctionTemplate(
            Name::Symbol::Full&& t_name
        ) : m_Name{ std::move(t_name) }
        {
        }
        virtual ~FunctionTemplate() = default;

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Template::Function* { return m_Symbol; }

    private:
        Symbol::Template::Function* m_Symbol{};
        Name::Symbol::Full m_Name{};
    };

    class AssociatedFunction : public virtual IBase
    {
    public:
        AssociatedFunction(
            const ITypeable& t_type,
            const char* const t_name,
            FunctionBodyEmitter&& t_bodyEmitter
        ) : m_Type{ t_type },
            m_Name{ t_name },
            m_BodyEmitter{ std::move(t_bodyEmitter) }
        {
        }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Function* { return m_Symbol; }

    private:
        Symbol::Function* m_Symbol{};
        const ITypeable& m_Type;
        const char* m_Name{};
        FunctionBodyEmitter m_BodyEmitter{};
    };

    class AssociatedFunctionTemplate : public virtual IBase
    {
    public:
        AssociatedFunctionTemplate(
            const ITypeable& t_type,
            const char* const t_name
        ) : m_Type{ t_type },
            m_Name{ t_name }
        {
        }
        virtual ~AssociatedFunctionTemplate() = default;

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Template::Function* { return m_Symbol; }

    private:
        Symbol::Template::Function* m_Symbol{};
        const ITypeable& m_Type;
        const char* m_Name{};
    };

    extern Type Int8;
    extern Type Int16;
    extern Type Int32;
    extern Type Int64;
    
    extern Type UInt8;
    extern Type UInt16;
    extern Type UInt32;
    extern Type UInt64;

    extern Type Int;
    
    extern Type Float32;
    extern Type Float64;

    extern Type Bool;
    extern Type Void;
    extern Type String;

    extern Type Pointer;

    extern TypeTemplate Reference;
    extern TypeTemplate StrongPointer;
    extern TypeTemplate WeakPointer;

    extern Function print_int;
    extern Function print_ptr;

    extern Function alloc;
    extern Function free;
    extern Function copy;

    extern AssociatedFunction i8__from_i16;
    extern AssociatedFunction i8__from_i32;
    extern AssociatedFunction i8__from_i64;
    extern AssociatedFunction i8__from_u8;
    extern AssociatedFunction i8__from_u16;
    extern AssociatedFunction i8__from_u32;
    extern AssociatedFunction i8__from_u64;
    extern AssociatedFunction i8__from_int;
    extern AssociatedFunction i8__from_f32;
    extern AssociatedFunction i8__from_f64;
    extern AssociatedFunction i8__unary_plus;
    extern AssociatedFunction i8__unary_negation;
    extern AssociatedFunction i8__one_complement;
    extern AssociatedFunction i8__multiplication;
    extern AssociatedFunction i8__division;
    extern AssociatedFunction i8__remainder;
    extern AssociatedFunction i8__addition;
    extern AssociatedFunction i8__subtraction;
    extern AssociatedFunction i8__right_shift;
    extern AssociatedFunction i8__left_shift;
    extern AssociatedFunction i8__less_than;
    extern AssociatedFunction i8__greater_than;
    extern AssociatedFunction i8__less_than_equals;
    extern AssociatedFunction i8__greater_than_equals;
    extern AssociatedFunction i8__equals;
    extern AssociatedFunction i8__not_equals;
    extern AssociatedFunction i8__AND;
    extern AssociatedFunction i8__XOR;
    extern AssociatedFunction i8__OR;

    extern AssociatedFunction i16__from_i8;
    extern AssociatedFunction i16__from_i32;
    extern AssociatedFunction i16__from_i64;
    extern AssociatedFunction i16__from_u8;
    extern AssociatedFunction i16__from_u16;
    extern AssociatedFunction i16__from_u32;
    extern AssociatedFunction i16__from_u64;
    extern AssociatedFunction i16__from_int;
    extern AssociatedFunction i16__from_f32;
    extern AssociatedFunction i16__from_f64;
    extern AssociatedFunction i16__unary_plus;
    extern AssociatedFunction i16__unary_negation;
    extern AssociatedFunction i16__one_complement;
    extern AssociatedFunction i16__multiplication;
    extern AssociatedFunction i16__division;
    extern AssociatedFunction i16__remainder;
    extern AssociatedFunction i16__addition;
    extern AssociatedFunction i16__subtraction;
    extern AssociatedFunction i16__right_shift;
    extern AssociatedFunction i16__left_shift;
    extern AssociatedFunction i16__less_than;
    extern AssociatedFunction i16__greater_than;
    extern AssociatedFunction i16__less_than_equals;
    extern AssociatedFunction i16__greater_than_equals;
    extern AssociatedFunction i16__equals;
    extern AssociatedFunction i16__not_equals;
    extern AssociatedFunction i16__AND;
    extern AssociatedFunction i16__XOR;
    extern AssociatedFunction i16__OR;

    extern AssociatedFunction i32__from_i8;
    extern AssociatedFunction i32__from_i16;
    extern AssociatedFunction i32__from_i64;
    extern AssociatedFunction i32__from_u8;
    extern AssociatedFunction i32__from_u16;
    extern AssociatedFunction i32__from_u32;
    extern AssociatedFunction i32__from_u64;
    extern AssociatedFunction i32__from_int;
    extern AssociatedFunction i32__from_f32;
    extern AssociatedFunction i32__from_f64;
    extern AssociatedFunction i32__unary_plus;
    extern AssociatedFunction i32__unary_negation;
    extern AssociatedFunction i32__one_complement;
    extern AssociatedFunction i32__multiplication;
    extern AssociatedFunction i32__division;
    extern AssociatedFunction i32__remainder;
    extern AssociatedFunction i32__addition;
    extern AssociatedFunction i32__subtraction;
    extern AssociatedFunction i32__right_shift;
    extern AssociatedFunction i32__left_shift;
    extern AssociatedFunction i32__less_than;
    extern AssociatedFunction i32__greater_than;
    extern AssociatedFunction i32__less_than_equals;
    extern AssociatedFunction i32__greater_than_equals;
    extern AssociatedFunction i32__equals;
    extern AssociatedFunction i32__not_equals;
    extern AssociatedFunction i32__AND;
    extern AssociatedFunction i32__XOR;
    extern AssociatedFunction i32__OR;

    extern AssociatedFunction i64__from_i8;
    extern AssociatedFunction i64__from_i16;
    extern AssociatedFunction i64__from_i32;
    extern AssociatedFunction i64__from_u8;
    extern AssociatedFunction i64__from_u16;
    extern AssociatedFunction i64__from_u32;
    extern AssociatedFunction i64__from_u64;
    extern AssociatedFunction i64__from_int;
    extern AssociatedFunction i64__from_f32;
    extern AssociatedFunction i64__from_f64;
    extern AssociatedFunction i64__unary_plus;
    extern AssociatedFunction i64__unary_negation;
    extern AssociatedFunction i64__one_complement;
    extern AssociatedFunction i64__multiplication;
    extern AssociatedFunction i64__division;
    extern AssociatedFunction i64__remainder;
    extern AssociatedFunction i64__addition;
    extern AssociatedFunction i64__subtraction;
    extern AssociatedFunction i64__right_shift;
    extern AssociatedFunction i64__left_shift;
    extern AssociatedFunction i64__less_than;
    extern AssociatedFunction i64__greater_than;
    extern AssociatedFunction i64__less_than_equals;
    extern AssociatedFunction i64__greater_than_equals;
    extern AssociatedFunction i64__equals;
    extern AssociatedFunction i64__not_equals;
    extern AssociatedFunction i64__AND;
    extern AssociatedFunction i64__XOR;
    extern AssociatedFunction i64__OR;

    extern AssociatedFunction u8__from_i8;
    extern AssociatedFunction u8__from_i16;
    extern AssociatedFunction u8__from_i32;
    extern AssociatedFunction u8__from_i64;
    extern AssociatedFunction u8__from_u16;
    extern AssociatedFunction u8__from_u32;
    extern AssociatedFunction u8__from_u64;
    extern AssociatedFunction u8__from_int;
    extern AssociatedFunction u8__from_f32;
    extern AssociatedFunction u8__from_f64;
    extern AssociatedFunction u8__unary_plus;
    extern AssociatedFunction u8__unary_negation;
    extern AssociatedFunction u8__one_complement;
    extern AssociatedFunction u8__multiplication;
    extern AssociatedFunction u8__division;
    extern AssociatedFunction u8__remainder;
    extern AssociatedFunction u8__addition;
    extern AssociatedFunction u8__subtraction;
    extern AssociatedFunction u8__right_shift;
    extern AssociatedFunction u8__left_shift;
    extern AssociatedFunction u8__less_than;
    extern AssociatedFunction u8__greater_than;
    extern AssociatedFunction u8__less_than_equals;
    extern AssociatedFunction u8__greater_than_equals;
    extern AssociatedFunction u8__equals;
    extern AssociatedFunction u8__not_equals;
    extern AssociatedFunction u8__AND;
    extern AssociatedFunction u8__XOR;
    extern AssociatedFunction u8__OR;

    extern AssociatedFunction u16__from_i8;
    extern AssociatedFunction u16__from_i16;
    extern AssociatedFunction u16__from_i32;
    extern AssociatedFunction u16__from_i64;
    extern AssociatedFunction u16__from_u8;
    extern AssociatedFunction u16__from_u32;
    extern AssociatedFunction u16__from_u64;
    extern AssociatedFunction u16__from_int;
    extern AssociatedFunction u16__from_f32;
    extern AssociatedFunction u16__from_f64;
    extern AssociatedFunction u16__unary_plus;
    extern AssociatedFunction u16__unary_negation;
    extern AssociatedFunction u16__one_complement;
    extern AssociatedFunction u16__multiplication;
    extern AssociatedFunction u16__division;
    extern AssociatedFunction u16__remainder;
    extern AssociatedFunction u16__addition;
    extern AssociatedFunction u16__subtraction;
    extern AssociatedFunction u16__right_shift;
    extern AssociatedFunction u16__left_shift;
    extern AssociatedFunction u16__less_than;
    extern AssociatedFunction u16__greater_than;
    extern AssociatedFunction u16__less_than_equals;
    extern AssociatedFunction u16__greater_than_equals;
    extern AssociatedFunction u16__equals;
    extern AssociatedFunction u16__not_equals;
    extern AssociatedFunction u16__AND;
    extern AssociatedFunction u16__XOR;
    extern AssociatedFunction u16__OR;

    extern AssociatedFunction u32__from_i8;
    extern AssociatedFunction u32__from_i16;
    extern AssociatedFunction u32__from_i32;
    extern AssociatedFunction u32__from_i64;
    extern AssociatedFunction u32__from_u8;
    extern AssociatedFunction u32__from_u16;
    extern AssociatedFunction u32__from_u64;
    extern AssociatedFunction u32__from_int;
    extern AssociatedFunction u32__from_f32;
    extern AssociatedFunction u32__from_f64;
    extern AssociatedFunction u32__unary_plus;
    extern AssociatedFunction u32__unary_negation;
    extern AssociatedFunction u32__one_complement;
    extern AssociatedFunction u32__multiplication;
    extern AssociatedFunction u32__division;
    extern AssociatedFunction u32__remainder;
    extern AssociatedFunction u32__addition;
    extern AssociatedFunction u32__subtraction;
    extern AssociatedFunction u32__right_shift;
    extern AssociatedFunction u32__left_shift;
    extern AssociatedFunction u32__less_than;
    extern AssociatedFunction u32__greater_than;
    extern AssociatedFunction u32__less_than_equals;
    extern AssociatedFunction u32__greater_than_equals;
    extern AssociatedFunction u32__equals;
    extern AssociatedFunction u32__not_equals;
    extern AssociatedFunction u32__AND;
    extern AssociatedFunction u32__XOR;
    extern AssociatedFunction u32__OR;

    extern AssociatedFunction u64__from_i8;
    extern AssociatedFunction u64__from_i16;
    extern AssociatedFunction u64__from_i32;
    extern AssociatedFunction u64__from_i64;
    extern AssociatedFunction u64__from_u8;
    extern AssociatedFunction u64__from_u16;
    extern AssociatedFunction u64__from_u32;
    extern AssociatedFunction u64__from_int;
    extern AssociatedFunction u64__from_f32;
    extern AssociatedFunction u64__from_f64;
    extern AssociatedFunction u64__unary_plus;
    extern AssociatedFunction u64__unary_negation;
    extern AssociatedFunction u64__one_complement;
    extern AssociatedFunction u64__multiplication;
    extern AssociatedFunction u64__division;
    extern AssociatedFunction u64__remainder;
    extern AssociatedFunction u64__addition;
    extern AssociatedFunction u64__subtraction;
    extern AssociatedFunction u64__right_shift;
    extern AssociatedFunction u64__left_shift;
    extern AssociatedFunction u64__less_than;
    extern AssociatedFunction u64__greater_than;
    extern AssociatedFunction u64__less_than_equals;
    extern AssociatedFunction u64__greater_than_equals;
    extern AssociatedFunction u64__equals;
    extern AssociatedFunction u64__not_equals;
    extern AssociatedFunction u64__AND;
    extern AssociatedFunction u64__XOR;
    extern AssociatedFunction u64__OR;

    extern AssociatedFunction int__from_i8;
    extern AssociatedFunction int__from_i16;
    extern AssociatedFunction int__from_i32;
    extern AssociatedFunction int__from_i64;
    extern AssociatedFunction int__from_u8;
    extern AssociatedFunction int__from_u16;
    extern AssociatedFunction int__from_u32;
    extern AssociatedFunction int__from_u64;
    extern AssociatedFunction int__from_f32;
    extern AssociatedFunction int__from_f64;
    extern AssociatedFunction int__unary_plus;
    extern AssociatedFunction int__unary_negation;
    extern AssociatedFunction int__one_complement;
    extern AssociatedFunction int__multiplication;
    extern AssociatedFunction int__division;
    extern AssociatedFunction int__remainder;
    extern AssociatedFunction int__addition;
    extern AssociatedFunction int__subtraction;
    extern AssociatedFunction int__right_shift;
    extern AssociatedFunction int__left_shift;
    extern AssociatedFunction int__less_than;
    extern AssociatedFunction int__greater_than;
    extern AssociatedFunction int__less_than_equals;
    extern AssociatedFunction int__greater_than_equals;
    extern AssociatedFunction int__equals;
    extern AssociatedFunction int__not_equals;
    extern AssociatedFunction int__AND;
    extern AssociatedFunction int__XOR;
    extern AssociatedFunction int__OR;

    extern AssociatedFunction f32__from_i8;
    extern AssociatedFunction f32__from_i16;
    extern AssociatedFunction f32__from_i32;
    extern AssociatedFunction f32__from_i64;
    extern AssociatedFunction f32__from_u8;
    extern AssociatedFunction f32__from_u16;
    extern AssociatedFunction f32__from_u32;
    extern AssociatedFunction f32__from_u64;
    extern AssociatedFunction f32__from_int;
    extern AssociatedFunction f32__from_f64;
    extern AssociatedFunction f32__unary_plus;
    extern AssociatedFunction f32__unary_negation;
    extern AssociatedFunction f32__multiplication;
    extern AssociatedFunction f32__division;
    extern AssociatedFunction f32__remainder;
    extern AssociatedFunction f32__addition;
    extern AssociatedFunction f32__subtraction;
    extern AssociatedFunction f32__less_than;
    extern AssociatedFunction f32__greater_than;
    extern AssociatedFunction f32__less_than_equals;
    extern AssociatedFunction f32__greater_than_equals;
    extern AssociatedFunction f32__equals;
    extern AssociatedFunction f32__not_equals;

    extern AssociatedFunction f64__from_i8;
    extern AssociatedFunction f64__from_i16;
    extern AssociatedFunction f64__from_i32;
    extern AssociatedFunction f64__from_i64;
    extern AssociatedFunction f64__from_u8;
    extern AssociatedFunction f64__from_u16;
    extern AssociatedFunction f64__from_u32;
    extern AssociatedFunction f64__from_u64;
    extern AssociatedFunction f64__from_int;
    extern AssociatedFunction f64__from_f32;
    extern AssociatedFunction f64__unary_plus;
    extern AssociatedFunction f64__unary_negation;
    extern AssociatedFunction f64__multiplication;
    extern AssociatedFunction f64__division;
    extern AssociatedFunction f64__remainder;
    extern AssociatedFunction f64__addition;
    extern AssociatedFunction f64__subtraction;
    extern AssociatedFunction f64__less_than;
    extern AssociatedFunction f64__greater_than;
    extern AssociatedFunction f64__less_than_equals;
    extern AssociatedFunction f64__greater_than_equals;
    extern AssociatedFunction f64__equals;
    extern AssociatedFunction f64__not_equals;

    extern AssociatedFunctionTemplate StrongPointer__new;
    extern AssociatedFunctionTemplate StrongPointer__get_value;

    auto InitializeSymbols() -> Expected<void>;

    auto GetIRTypeSymbolMap(Emitter& t_emitter) -> std::unordered_map<Symbol::Type::IBase*, llvm::Type*>;
    auto GetImplicitFromOperatorMap() -> std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>;
    auto GetExplicitFromOperatorMap() -> std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>;
}
