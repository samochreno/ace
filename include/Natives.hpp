#pragma once

#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "LLVM.hpp"
#include "Error.hpp"
#include "Token.hpp"
#include "Scope.hpp"
#include "Error.hpp"

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

namespace Ace
{
    class Emitter;

    using FunctionBodyEmitter = std::function<void(Emitter&)>;

    class INative
    {
    public:
        virtual ~INative() = default;

        virtual auto GetCompilation() const -> const Compilation& = 0;

        virtual auto Initialize() -> Expected<void> = 0;
    };

    class ITypeableNative : public virtual INative
    {
    public:
        virtual ~ITypeableNative() = default;

        virtual auto GetFullyQualifiedName() const -> const SymbolName& = 0;
    };

    enum class NativeCopyabilityKind
    {
        None,
        Trivial,
        NonTrivial,
    };

    enum class NativeSizeKind
    {
        None,
        Sized,
        Unsized,
    };

    class NativeType : public virtual ITypeableNative
    {
    public:
        NativeType(
            const Compilation& t_compilation,
            SymbolName&& t_name,
            std::optional<std::function<llvm::Type*()>>&& t_irTypeGetter,
            const NativeSizeKind& t_sizeKind,
            const NativeCopyabilityKind& t_copyabilityKind
        ) : m_Compilation{ t_compilation },
            m_Name{ std::move(t_name) },
            m_IRTypeGetter{ std::move(t_irTypeGetter) },
            m_IsSized{ t_sizeKind == NativeSizeKind::Sized },
            m_IsTriviallyCopyable{ t_copyabilityKind == NativeCopyabilityKind::Trivial }
        {
            ACE_ASSERT(
                (t_sizeKind == NativeSizeKind::Sized) ||
                (t_sizeKind == NativeSizeKind::Unsized)
            );

            ACE_ASSERT(
                (t_copyabilityKind == NativeCopyabilityKind::Trivial) ||
                (t_copyabilityKind == NativeCopyabilityKind::NonTrivial)
            );
        }
        ~NativeType() = default;

        auto GetFullyQualifiedName() const -> const SymbolName& final { return m_Name; }

        auto GetCompilation() const -> const Compilation& { return m_Compilation; }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Type::IBase*
        {
            ACE_ASSERT(m_Symbol);
            return m_Symbol;
        }

        auto HasIRType() const -> bool { return m_IRTypeGetter.has_value(); }
        auto GetIRType() const -> llvm::Type*
        {
            ACE_ASSERT(HasIRType());
            return m_IRTypeGetter.value()();
        }

    private:
        const Compilation& m_Compilation;
        SymbolName m_Name{};
        std::optional<std::function<llvm::Type*()>> m_IRTypeGetter{};
        bool m_IsSized{};
        bool m_IsTriviallyCopyable{};

        Symbol::Type::IBase* m_Symbol{};
    };

    class NativeTypeTemplate : public virtual ITypeableNative
    {
    public:
        NativeTypeTemplate(
            const Compilation& t_compilation,
            SymbolName&& t_name
        ) : m_Compilation{ t_compilation },
            m_Name{ std::move(t_name) }
        {
        }
        ~NativeTypeTemplate() = default;

        auto GetFullyQualifiedName() const -> const SymbolName& final { return m_Name; }

        auto GetCompilation() const -> const Compilation& { return m_Compilation; }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Template::Type*
        {
            ACE_ASSERT(m_Symbol);
            return m_Symbol;
        }

    private:
        const Compilation& m_Compilation;
        SymbolName m_Name{};

        Symbol::Template::Type* m_Symbol{};
    };
    
    class NativeFunction : public virtual INative
    {
    public:
        NativeFunction(
            const Compilation& t_compilation,
            SymbolName&& t_name,
            FunctionBodyEmitter&& t_bodyEmitter
        ) : m_Compilation{ t_compilation },
            m_Name{ std::move(t_name) },
            m_BodyEmitter{ std::move(t_bodyEmitter) }
        {
        }
        ~NativeFunction() = default;

        auto GetCompilation() const -> const Compilation& { return m_Compilation; }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Function*
        {
            ACE_ASSERT(m_Symbol);
            return m_Symbol;
        }

    private:
        const Compilation& m_Compilation;
        SymbolName m_Name{};
        FunctionBodyEmitter m_BodyEmitter{};

        Symbol::Function* m_Symbol{};
    };

    class NativeFunctionTemplate : public virtual INative
    {
    public:
        NativeFunctionTemplate(
            const Compilation& t_compilation,
            SymbolName&& t_name
        ) : m_Compilation{ t_compilation },
            m_Name{ std::move(t_name) }
        {
        }
        ~NativeFunctionTemplate() = default;

        auto GetCompilation() const -> const Compilation& { return m_Compilation; }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Template::Function*
        {
            ACE_ASSERT(m_Symbol);
            return m_Symbol;
        }

    private:
        const Compilation& m_Compilation;
        SymbolName m_Name{};

        Symbol::Template::Function* m_Symbol{};
    };

    class NativeAssociatedFunction : public virtual INative
    {
    public:
        NativeAssociatedFunction(
            const ITypeableNative& t_type,
            const char* const t_name,
            FunctionBodyEmitter&& t_bodyEmitter
        ) : m_Type{ t_type },
            m_Name{ t_name },
            m_BodyEmitter{ std::move(t_bodyEmitter) }
        {
        }
        ~NativeAssociatedFunction() = default;

        auto GetCompilation() const -> const Compilation& { return m_Type.GetCompilation(); }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Function*
        {
            ACE_ASSERT(m_Symbol);
            return m_Symbol;
        }

    private:
        const ITypeableNative& m_Type;
        const char* m_Name{};
        FunctionBodyEmitter m_BodyEmitter{};

        Symbol::Function* m_Symbol{};
    };

    class NativeAssociatedFunctionTemplate : public virtual INative
    {
    public:
        NativeAssociatedFunctionTemplate(
            const ITypeableNative& t_type,
            const char* const t_name
        ) : m_Type{ t_type },
            m_Name{ t_name }
        {
        }
        ~NativeAssociatedFunctionTemplate() = default;

        auto GetCompilation() const -> const Compilation& { return m_Type.GetCompilation(); }

        auto Initialize() -> Expected<void> final;

        auto GetSymbol() const -> Symbol::Template::Function*
        {
            ACE_ASSERT(m_Symbol);
            return m_Symbol;
        }

    private:
        const ITypeableNative& m_Type;
        const char* m_Name{};

        Symbol::Template::Function* m_Symbol{};
    };

    class Natives
    {
    public:
        Natives(const Compilation& t_compilation);
        ~Natives() = default;

        auto Initialize() -> Expected<void>;

        auto GetIRTypeSymbolMap() const -> const std::unordered_map<Symbol::Type::IBase*, llvm::Type*>& { return m_IRTypeSymbolMap; }
        auto GetImplicitFromOperatorMap() const -> const std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>& { return m_ImplicitFromOperatorMap; }
        auto GetExplicitFromOperatorMap() const -> const std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>& { return m_ExplicitFromOperatorMap; }

        auto IsIntTypeSigned(const NativeType& t_intType) const -> bool;

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
        NativeType Void;
        NativeType String;

        NativeType Pointer;

        NativeTypeTemplate Reference;
        NativeTypeTemplate StrongPointer;
        NativeTypeTemplate WeakPointer;

        NativeFunction print_int;
        NativeFunction print_ptr;

        NativeFunction alloc;
        NativeFunction dealloc;
        NativeFunction copy;

        NativeAssociatedFunction i8__from_i16;
        NativeAssociatedFunction i8__from_i32;
        NativeAssociatedFunction i8__from_i64;
        NativeAssociatedFunction i8__from_u8;
        NativeAssociatedFunction i8__from_u16;
        NativeAssociatedFunction i8__from_u32;
        NativeAssociatedFunction i8__from_u64;
        NativeAssociatedFunction i8__from_int;
        NativeAssociatedFunction i8__from_f32;
        NativeAssociatedFunction i8__from_f64;
        NativeAssociatedFunction i8__unary_plus;
        NativeAssociatedFunction i8__unary_negation;
        NativeAssociatedFunction i8__one_complement;
        NativeAssociatedFunction i8__multiplication;
        NativeAssociatedFunction i8__division;
        NativeAssociatedFunction i8__remainder;
        NativeAssociatedFunction i8__addition;
        NativeAssociatedFunction i8__subtraction;
        NativeAssociatedFunction i8__right_shift;
        NativeAssociatedFunction i8__left_shift;
        NativeAssociatedFunction i8__less_than;
        NativeAssociatedFunction i8__greater_than;
        NativeAssociatedFunction i8__less_than_equals;
        NativeAssociatedFunction i8__greater_than_equals;
        NativeAssociatedFunction i8__equals;
        NativeAssociatedFunction i8__not_equals;
        NativeAssociatedFunction i8__AND;
        NativeAssociatedFunction i8__XOR;
        NativeAssociatedFunction i8__OR;

        NativeAssociatedFunction i16__from_i8;
        NativeAssociatedFunction i16__from_i32;
        NativeAssociatedFunction i16__from_i64;
        NativeAssociatedFunction i16__from_u8;
        NativeAssociatedFunction i16__from_u16;
        NativeAssociatedFunction i16__from_u32;
        NativeAssociatedFunction i16__from_u64;
        NativeAssociatedFunction i16__from_int;
        NativeAssociatedFunction i16__from_f32;
        NativeAssociatedFunction i16__from_f64;
        NativeAssociatedFunction i16__unary_plus;
        NativeAssociatedFunction i16__unary_negation;
        NativeAssociatedFunction i16__one_complement;
        NativeAssociatedFunction i16__multiplication;
        NativeAssociatedFunction i16__division;
        NativeAssociatedFunction i16__remainder;
        NativeAssociatedFunction i16__addition;
        NativeAssociatedFunction i16__subtraction;
        NativeAssociatedFunction i16__right_shift;
        NativeAssociatedFunction i16__left_shift;
        NativeAssociatedFunction i16__less_than;
        NativeAssociatedFunction i16__greater_than;
        NativeAssociatedFunction i16__less_than_equals;
        NativeAssociatedFunction i16__greater_than_equals;
        NativeAssociatedFunction i16__equals;
        NativeAssociatedFunction i16__not_equals;
        NativeAssociatedFunction i16__AND;
        NativeAssociatedFunction i16__XOR;
        NativeAssociatedFunction i16__OR;

        NativeAssociatedFunction i32__from_i8;
        NativeAssociatedFunction i32__from_i16;
        NativeAssociatedFunction i32__from_i64;
        NativeAssociatedFunction i32__from_u8;
        NativeAssociatedFunction i32__from_u16;
        NativeAssociatedFunction i32__from_u32;
        NativeAssociatedFunction i32__from_u64;
        NativeAssociatedFunction i32__from_int;
        NativeAssociatedFunction i32__from_f32;
        NativeAssociatedFunction i32__from_f64;
        NativeAssociatedFunction i32__unary_plus;
        NativeAssociatedFunction i32__unary_negation;
        NativeAssociatedFunction i32__one_complement;
        NativeAssociatedFunction i32__multiplication;
        NativeAssociatedFunction i32__division;
        NativeAssociatedFunction i32__remainder;
        NativeAssociatedFunction i32__addition;
        NativeAssociatedFunction i32__subtraction;
        NativeAssociatedFunction i32__right_shift;
        NativeAssociatedFunction i32__left_shift;
        NativeAssociatedFunction i32__less_than;
        NativeAssociatedFunction i32__greater_than;
        NativeAssociatedFunction i32__less_than_equals;
        NativeAssociatedFunction i32__greater_than_equals;
        NativeAssociatedFunction i32__equals;
        NativeAssociatedFunction i32__not_equals;
        NativeAssociatedFunction i32__AND;
        NativeAssociatedFunction i32__XOR;
        NativeAssociatedFunction i32__OR;

        NativeAssociatedFunction i64__from_i8;
        NativeAssociatedFunction i64__from_i16;
        NativeAssociatedFunction i64__from_i32;
        NativeAssociatedFunction i64__from_u8;
        NativeAssociatedFunction i64__from_u16;
        NativeAssociatedFunction i64__from_u32;
        NativeAssociatedFunction i64__from_u64;
        NativeAssociatedFunction i64__from_int;
        NativeAssociatedFunction i64__from_f32;
        NativeAssociatedFunction i64__from_f64;
        NativeAssociatedFunction i64__unary_plus;
        NativeAssociatedFunction i64__unary_negation;
        NativeAssociatedFunction i64__one_complement;
        NativeAssociatedFunction i64__multiplication;
        NativeAssociatedFunction i64__division;
        NativeAssociatedFunction i64__remainder;
        NativeAssociatedFunction i64__addition;
        NativeAssociatedFunction i64__subtraction;
        NativeAssociatedFunction i64__right_shift;
        NativeAssociatedFunction i64__left_shift;
        NativeAssociatedFunction i64__less_than;
        NativeAssociatedFunction i64__greater_than;
        NativeAssociatedFunction i64__less_than_equals;
        NativeAssociatedFunction i64__greater_than_equals;
        NativeAssociatedFunction i64__equals;
        NativeAssociatedFunction i64__not_equals;
        NativeAssociatedFunction i64__AND;
        NativeAssociatedFunction i64__XOR;
        NativeAssociatedFunction i64__OR;

        NativeAssociatedFunction u8__from_i8;
        NativeAssociatedFunction u8__from_i16;
        NativeAssociatedFunction u8__from_i32;
        NativeAssociatedFunction u8__from_i64;
        NativeAssociatedFunction u8__from_u16;
        NativeAssociatedFunction u8__from_u32;
        NativeAssociatedFunction u8__from_u64;
        NativeAssociatedFunction u8__from_int;
        NativeAssociatedFunction u8__from_f32;
        NativeAssociatedFunction u8__from_f64;
        NativeAssociatedFunction u8__unary_plus;
        NativeAssociatedFunction u8__unary_negation;
        NativeAssociatedFunction u8__one_complement;
        NativeAssociatedFunction u8__multiplication;
        NativeAssociatedFunction u8__division;
        NativeAssociatedFunction u8__remainder;
        NativeAssociatedFunction u8__addition;
        NativeAssociatedFunction u8__subtraction;
        NativeAssociatedFunction u8__right_shift;
        NativeAssociatedFunction u8__left_shift;
        NativeAssociatedFunction u8__less_than;
        NativeAssociatedFunction u8__greater_than;
        NativeAssociatedFunction u8__less_than_equals;
        NativeAssociatedFunction u8__greater_than_equals;
        NativeAssociatedFunction u8__equals;
        NativeAssociatedFunction u8__not_equals;
        NativeAssociatedFunction u8__AND;
        NativeAssociatedFunction u8__XOR;
        NativeAssociatedFunction u8__OR;

        NativeAssociatedFunction u16__from_i8;
        NativeAssociatedFunction u16__from_i16;
        NativeAssociatedFunction u16__from_i32;
        NativeAssociatedFunction u16__from_i64;
        NativeAssociatedFunction u16__from_u8;
        NativeAssociatedFunction u16__from_u32;
        NativeAssociatedFunction u16__from_u64;
        NativeAssociatedFunction u16__from_int;
        NativeAssociatedFunction u16__from_f32;
        NativeAssociatedFunction u16__from_f64;
        NativeAssociatedFunction u16__unary_plus;
        NativeAssociatedFunction u16__unary_negation;
        NativeAssociatedFunction u16__one_complement;
        NativeAssociatedFunction u16__multiplication;
        NativeAssociatedFunction u16__division;
        NativeAssociatedFunction u16__remainder;
        NativeAssociatedFunction u16__addition;
        NativeAssociatedFunction u16__subtraction;
        NativeAssociatedFunction u16__right_shift;
        NativeAssociatedFunction u16__left_shift;
        NativeAssociatedFunction u16__less_than;
        NativeAssociatedFunction u16__greater_than;
        NativeAssociatedFunction u16__less_than_equals;
        NativeAssociatedFunction u16__greater_than_equals;
        NativeAssociatedFunction u16__equals;
        NativeAssociatedFunction u16__not_equals;
        NativeAssociatedFunction u16__AND;
        NativeAssociatedFunction u16__XOR;
        NativeAssociatedFunction u16__OR;

        NativeAssociatedFunction u32__from_i8;
        NativeAssociatedFunction u32__from_i16;
        NativeAssociatedFunction u32__from_i32;
        NativeAssociatedFunction u32__from_i64;
        NativeAssociatedFunction u32__from_u8;
        NativeAssociatedFunction u32__from_u16;
        NativeAssociatedFunction u32__from_u64;
        NativeAssociatedFunction u32__from_int;
        NativeAssociatedFunction u32__from_f32;
        NativeAssociatedFunction u32__from_f64;
        NativeAssociatedFunction u32__unary_plus;
        NativeAssociatedFunction u32__unary_negation;
        NativeAssociatedFunction u32__one_complement;
        NativeAssociatedFunction u32__multiplication;
        NativeAssociatedFunction u32__division;
        NativeAssociatedFunction u32__remainder;
        NativeAssociatedFunction u32__addition;
        NativeAssociatedFunction u32__subtraction;
        NativeAssociatedFunction u32__right_shift;
        NativeAssociatedFunction u32__left_shift;
        NativeAssociatedFunction u32__less_than;
        NativeAssociatedFunction u32__greater_than;
        NativeAssociatedFunction u32__less_than_equals;
        NativeAssociatedFunction u32__greater_than_equals;
        NativeAssociatedFunction u32__equals;
        NativeAssociatedFunction u32__not_equals;
        NativeAssociatedFunction u32__AND;
        NativeAssociatedFunction u32__XOR;
        NativeAssociatedFunction u32__OR;

        NativeAssociatedFunction u64__from_i8;
        NativeAssociatedFunction u64__from_i16;
        NativeAssociatedFunction u64__from_i32;
        NativeAssociatedFunction u64__from_i64;
        NativeAssociatedFunction u64__from_u8;
        NativeAssociatedFunction u64__from_u16;
        NativeAssociatedFunction u64__from_u32;
        NativeAssociatedFunction u64__from_int;
        NativeAssociatedFunction u64__from_f32;
        NativeAssociatedFunction u64__from_f64;
        NativeAssociatedFunction u64__unary_plus;
        NativeAssociatedFunction u64__unary_negation;
        NativeAssociatedFunction u64__one_complement;
        NativeAssociatedFunction u64__multiplication;
        NativeAssociatedFunction u64__division;
        NativeAssociatedFunction u64__remainder;
        NativeAssociatedFunction u64__addition;
        NativeAssociatedFunction u64__subtraction;
        NativeAssociatedFunction u64__right_shift;
        NativeAssociatedFunction u64__left_shift;
        NativeAssociatedFunction u64__less_than;
        NativeAssociatedFunction u64__greater_than;
        NativeAssociatedFunction u64__less_than_equals;
        NativeAssociatedFunction u64__greater_than_equals;
        NativeAssociatedFunction u64__equals;
        NativeAssociatedFunction u64__not_equals;
        NativeAssociatedFunction u64__AND;
        NativeAssociatedFunction u64__XOR;
        NativeAssociatedFunction u64__OR;

        NativeAssociatedFunction int__from_i8;
        NativeAssociatedFunction int__from_i16;
        NativeAssociatedFunction int__from_i32;
        NativeAssociatedFunction int__from_i64;
        NativeAssociatedFunction int__from_u8;
        NativeAssociatedFunction int__from_u16;
        NativeAssociatedFunction int__from_u32;
        NativeAssociatedFunction int__from_u64;
        NativeAssociatedFunction int__from_f32;
        NativeAssociatedFunction int__from_f64;
        NativeAssociatedFunction int__unary_plus;
        NativeAssociatedFunction int__unary_negation;
        NativeAssociatedFunction int__one_complement;
        NativeAssociatedFunction int__multiplication;
        NativeAssociatedFunction int__division;
        NativeAssociatedFunction int__remainder;
        NativeAssociatedFunction int__addition;
        NativeAssociatedFunction int__subtraction;
        NativeAssociatedFunction int__right_shift;
        NativeAssociatedFunction int__left_shift;
        NativeAssociatedFunction int__less_than;
        NativeAssociatedFunction int__greater_than;
        NativeAssociatedFunction int__less_than_equals;
        NativeAssociatedFunction int__greater_than_equals;
        NativeAssociatedFunction int__equals;
        NativeAssociatedFunction int__not_equals;
        NativeAssociatedFunction int__AND;
        NativeAssociatedFunction int__XOR;
        NativeAssociatedFunction int__OR;

        NativeAssociatedFunction f32__from_i8;
        NativeAssociatedFunction f32__from_i16;
        NativeAssociatedFunction f32__from_i32;
        NativeAssociatedFunction f32__from_i64;
        NativeAssociatedFunction f32__from_u8;
        NativeAssociatedFunction f32__from_u16;
        NativeAssociatedFunction f32__from_u32;
        NativeAssociatedFunction f32__from_u64;
        NativeAssociatedFunction f32__from_int;
        NativeAssociatedFunction f32__from_f64;
        NativeAssociatedFunction f32__unary_plus;
        NativeAssociatedFunction f32__unary_negation;
        NativeAssociatedFunction f32__multiplication;
        NativeAssociatedFunction f32__division;
        NativeAssociatedFunction f32__remainder;
        NativeAssociatedFunction f32__addition;
        NativeAssociatedFunction f32__subtraction;
        NativeAssociatedFunction f32__less_than;
        NativeAssociatedFunction f32__greater_than;
        NativeAssociatedFunction f32__less_than_equals;
        NativeAssociatedFunction f32__greater_than_equals;
        NativeAssociatedFunction f32__equals;
        NativeAssociatedFunction f32__not_equals;

        NativeAssociatedFunction f64__from_i8;
        NativeAssociatedFunction f64__from_i16;
        NativeAssociatedFunction f64__from_i32;
        NativeAssociatedFunction f64__from_i64;
        NativeAssociatedFunction f64__from_u8;
        NativeAssociatedFunction f64__from_u16;
        NativeAssociatedFunction f64__from_u32;
        NativeAssociatedFunction f64__from_u64;
        NativeAssociatedFunction f64__from_int;
        NativeAssociatedFunction f64__from_f32;
        NativeAssociatedFunction f64__unary_plus;
        NativeAssociatedFunction f64__unary_negation;
        NativeAssociatedFunction f64__multiplication;
        NativeAssociatedFunction f64__division;
        NativeAssociatedFunction f64__remainder;
        NativeAssociatedFunction f64__addition;
        NativeAssociatedFunction f64__subtraction;
        NativeAssociatedFunction f64__less_than;
        NativeAssociatedFunction f64__greater_than;
        NativeAssociatedFunction f64__less_than_equals;
        NativeAssociatedFunction f64__greater_than_equals;
        NativeAssociatedFunction f64__equals;
        NativeAssociatedFunction f64__not_equals;

        NativeAssociatedFunctionTemplate StrongPointer__new;
        NativeAssociatedFunctionTemplate StrongPointer__value;

    private:
        auto InitializeCollectionsOfNatives() -> void;

        auto InitializeIRTypeSymbolMap() -> void;

        auto InitializeImplicitFromOperatorMap() -> void;
        auto InitializeExplicitFromOperatorMap() -> void;

        auto InitializeSignedIntTypesSet() -> void;

        std::vector<INative*> m_Natives{};
        std::vector<NativeType*> m_Types{};
        std::vector<NativeTypeTemplate*> m_TypeTemplates{};
        std::vector<NativeFunction*> m_Functions{};
        std::vector<NativeFunctionTemplate*> m_FunctionTemplates{};
        std::vector<NativeAssociatedFunction*> m_AssociatedFunctions{};
        std::vector<NativeAssociatedFunctionTemplate*> m_AssociatedFunctionTemplates{};

        std::unordered_map<Symbol::Type::IBase*, llvm::Type*> m_IRTypeSymbolMap{};

        std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>> m_ImplicitFromOperatorMap{};
        std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>> m_ExplicitFromOperatorMap{};

        std::unordered_set<const NativeType*> m_SignedIntTypesSet{};
    };
}

