#include "Natives.hpp"

#include <vector>
#include <optional>
#include <algorithm>

#include "Diagnostics.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "Emittable.hpp"
#include "Scope.hpp"
#include "Emitter.hpp"
#include "SpecialIdentifier.hpp"
#include "Diagnostics.hpp"

namespace Ace
{
    class FunctionEmittableBody : public IEmittable<void>
    {
    public:
        FunctionEmittableBody(const FunctionBodyEmitter& t_bodyEmitter)
            : m_BodyEmitter{ t_bodyEmitter }
        {
        }
        virtual ~FunctionEmittableBody() = default;

        auto Emit(Emitter& t_emitter) const -> void final
        {
            m_BodyEmitter(t_emitter);
        }

   private:
        FunctionBodyEmitter m_BodyEmitter;
    };

    NativeType::NativeType(
        const Compilation* const t_compilation,
        SymbolName&& t_name,
        std::optional<std::function<llvm::Type*()>>&& t_irTypeGetter,
        const TypeSizeKind& t_sizeKind,
        const NativeCopyabilityKind& t_copyabilityKind
    ) : m_Compilation{ t_compilation },
        m_Name{ std::move(t_name) },
        m_IRTypeGetter{ std::move(t_irTypeGetter) },
        m_IsSized{ t_sizeKind == TypeSizeKind::Sized },
        m_IsTriviallyCopyable{ t_copyabilityKind == NativeCopyabilityKind::Trivial }
    {
        ACE_ASSERT(
            (t_sizeKind == TypeSizeKind::Sized) ||
            (t_sizeKind == TypeSizeKind::Unsized)
        );

        ACE_ASSERT(
            (t_copyabilityKind == NativeCopyabilityKind::Trivial) ||
            (t_copyabilityKind == NativeCopyabilityKind::NonTrivial)
        );
    }

    auto NativeType::GetFullyQualifiedName() const -> const SymbolName&
    {
        return m_Name;
    }

    auto NativeType::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    auto NativeType::GetSymbol() const -> ITypeSymbol*
    {
        ACE_ASSERT(m_Symbol);
        return m_Symbol;
    }

    auto NativeType::HasIRType() const -> bool
    {
        return m_IRTypeGetter.has_value();
    }

    auto NativeType::GetIRType() const -> llvm::Type*
    {
        ACE_ASSERT(HasIRType());
        return m_IRTypeGetter.value()();
    }

    auto NativeType::Initialize() -> void
    {
        auto* const symbol =
            GetCompilation()->GlobalScope.Unwrap()->ResolveStaticSymbol<ITypeSymbol>(m_Name).Unwrap();

        if (m_IRTypeGetter.has_value())
        {
            symbol->SetAsPrimitivelyEmittable();
        }

        if (!m_IsSized)
        {
            symbol->SetAsUnsized();
        }

        if (m_IsTriviallyCopyable)
        {
            symbol->SetAsTriviallyCopyable();
        }

        m_Symbol = symbol;
    }

    auto NativeTypeTemplate::Initialize() -> void
    {
        auto name = m_Name;
        name.Sections.back().Name = SpecialIdentifier::CreateTemplate(
            name.Sections.back().Name
        );

        auto* const symbol =
            GetCompilation()->GlobalScope.Unwrap()->ResolveStaticSymbol<TypeTemplateSymbol>(name).Unwrap();

        m_Symbol = symbol;
    }

    NativeTypeTemplate::NativeTypeTemplate(
        const Compilation* const t_compilation,
        SymbolName&& t_name
    ) : m_Compilation{ t_compilation },
        m_Name{ std::move(t_name) }
    {
    }

    auto NativeTypeTemplate::GetFullyQualifiedName() const -> const SymbolName&
    {
        return m_Name;
    }

    auto NativeTypeTemplate::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    auto NativeTypeTemplate::GetSymbol() const -> TypeTemplateSymbol*
    {
        ACE_ASSERT(m_Symbol);
        return m_Symbol;
    }

    NativeFunction::NativeFunction(
        const Compilation* const t_compilation,
        SymbolName&& t_name,
        FunctionBodyEmitter&& t_bodyEmitter
    ) : m_Compilation{ t_compilation },
        m_Name{ std::move(t_name) },
        m_BodyEmitter{ std::move(t_bodyEmitter) }
    {
    }

    auto NativeFunction::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    auto NativeFunction::GetSymbol() const -> FunctionSymbol*
    {
        ACE_ASSERT(m_Symbol);
        return m_Symbol;
    }

    auto NativeFunction::Initialize() -> void
    {
        auto* const symbol =
            GetCompilation()->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionSymbol>(m_Name).Unwrap();

        symbol->BindBody(std::make_shared<FunctionEmittableBody>(
            m_BodyEmitter
        ));

        m_Symbol = symbol;
    }

    NativeFunctionTemplate::NativeFunctionTemplate(
        const Compilation* const t_compilation,
        SymbolName&& t_name
    ) : m_Compilation{ t_compilation },
        m_Name{ std::move(t_name) }
    {
    }

    auto NativeFunctionTemplate::Initialize() -> void
    {
        auto* const symbol =
            GetCompilation()->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionTemplateSymbol>(m_Name).Unwrap();

        m_Symbol = symbol;
    }

    auto NativeFunctionTemplate::GetCompilation() const -> const Compilation*
    {
        return m_Compilation;
    }

    NativeAssociatedFunction::NativeAssociatedFunction(
        const ITypeableNative& t_type,
        const char* const t_name,
        FunctionBodyEmitter&& t_bodyEmitter
    ) : m_Type{ t_type },
        m_Name{ t_name },
        m_BodyEmitter{ std::move(t_bodyEmitter) }
    {
    }

    auto NativeAssociatedFunction::GetCompilation() const -> const Compilation*
    {
        return m_Type.GetCompilation();
    }

    auto NativeAssociatedFunction::Initialize() -> void
    {
        auto name = m_Type.GetFullyQualifiedName();
        name.Sections.emplace_back(m_Name);

        auto* const symbol =
            GetCompilation()->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionSymbol>(name).Unwrap();

        symbol->BindBody(std::make_shared<FunctionEmittableBody>(
            m_BodyEmitter
        ));

        m_Symbol = symbol;
    }

    auto NativeAssociatedFunction::GetSymbol() const -> FunctionSymbol*
    {
        ACE_ASSERT(m_Symbol);
        return m_Symbol;
    }

    NativeAssociatedFunctionTemplate::NativeAssociatedFunctionTemplate(
        const ITypeableNative& t_type,
        const char* const t_name
    ) : m_Type{ t_type },
        m_Name{ t_name }
    {
    }

    auto NativeAssociatedFunctionTemplate::GetCompilation() const -> const Compilation*
    {
        return m_Type.GetCompilation();
    }

    auto NativeAssociatedFunctionTemplate::Initialize() -> void
    {
        auto name = m_Type.GetFullyQualifiedName();
        name.Sections.emplace_back(m_Name);

        auto* const symbol =
            GetCompilation()->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionTemplateSymbol>(name).Unwrap();

        m_Symbol = symbol;
    }

    auto NativeAssociatedFunctionTemplate::GetSymbol() const -> FunctionTemplateSymbol*
    {
        ACE_ASSERT(m_Symbol);
        return m_Symbol;
    }

    namespace I
    {        
        static auto FromIntImpl(
            const char* const t_name,
            const NativeType& t_fromType,
            const NativeType& t_toType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_toType,
                t_name,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (t_fromType.GetCompilation()->Natives->IsIntTypeSigned(t_fromType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSExtOrTrunc(
                                t_emitter.EmitLoadArg(0, t_fromType.GetIRType()),
                                t_toType.GetIRType()
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateZExtOrTrunc(
                                t_emitter.EmitLoadArg(0, t_fromType.GetIRType()),
                                t_toType.GetIRType()
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto FromInt8(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i8",
                t_natives->Int8,
                t_toType
            );
        }

        static auto FromInt16(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i16",
                t_natives->Int16,
                t_toType
            );
        }

        static auto FromInt32(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i32",
                t_natives->Int32,
                t_toType
            );
        }

        static auto FromInt64(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i64", 
                t_natives->Int64,
                t_toType
            );
        }

        static auto FromUInt8(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u8",
                t_natives->UInt8,
                t_toType
            );
        }

        static auto FromUInt16(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u16",
                t_natives->UInt16,
                t_toType
            );
        }

        static auto FromUInt32(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u32",
                t_natives->UInt32,
                t_toType
            );
        }

        static auto FromUInt64(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u64",
                t_natives->UInt64,
                t_toType
            );
        }

        static auto FromInt(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_int",
                t_natives->Int,
                t_toType
            );
        }

        static auto FromFloat(
            const char* const t_name,
            const NativeType& t_fromType,
            const NativeType& t_toType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_toType,
                t_name,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (t_fromType.GetCompilation()->Natives->IsIntTypeSigned(t_toType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateFPToSI(
                                t_emitter.EmitLoadArg(0, t_fromType.GetIRType()),
                                t_toType.GetIRType()
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateFPToUI(
                                t_emitter.EmitLoadArg(0, t_fromType.GetIRType()),
                                t_toType.GetIRType()
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto FromFloat32(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromFloat(
                "from_f32",
                t_natives->Float32,
                t_toType
            );
        }

        static auto FromFloat64(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromFloat(
                "from_f64",
                t_natives->Float64,
                t_toType
            );
        }

        static auto Division(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Division,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (t_selfType.GetCompilation()->Natives->IsIntTypeSigned(t_selfType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSDiv(
                                t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                                t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateUDiv(
                                t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                                t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Remainder(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Remainder,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (t_selfType.GetCompilation()->Natives->IsIntTypeSigned(t_selfType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSRem(
                                t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                                t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateURem(
                                t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                                t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto UnaryPlus(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryPlus,
                [&](Emitter& t_emitter)
                {
                    t_emitter.GetBlockBuilder().Builder.CreateRet(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType())
                    );
                }
            };
        }

        static auto UnaryNegation(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryNegation,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateMul(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        llvm::ConstantInt::get(t_selfType.GetIRType(), -1)
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };

        }

        static auto OneComplement(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::OneComplement,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateXor(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        llvm::ConstantInt::get(t_selfType.GetIRType(), -1)
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Multiplication(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return 
            {
                t_selfType,
                SpecialIdentifier::Operator::Multiplication,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateMul(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Addition(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Addition,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateAdd(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Subtraction(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Subtraction,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateSub(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto RightShift(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::RightShift,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateAShr(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        };

        static auto LeftShift(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LeftShift,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateShl(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThan(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLT,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThan(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGT,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThanEquals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLE,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThanEquals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGE,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Equals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Equals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_EQ,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto NotEquals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::NotEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_NE,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto AND(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::AND,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateAnd(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto XOR(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::XOR,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateXor(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto OR(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::OR,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateOr(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }
    }

    namespace F
    {
        static auto FromIntImpl(
            const char* const t_name,
            const NativeType& t_fromType,
            const NativeType& t_toType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_toType,
                t_name,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (t_fromType.GetCompilation()->Natives->IsIntTypeSigned(t_fromType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSIToFP(
                                t_emitter.EmitLoadArg(0, t_fromType.GetIRType()),
                                t_toType.GetIRType()
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateUIToFP(
                                t_emitter.EmitLoadArg(0, t_fromType.GetIRType()),
                                t_toType.GetIRType()
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto FromInt8(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i8",
                t_natives->Int8,
                t_toType
            );
        }

        static auto FromInt16(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i16",
                t_natives->Int16,
                t_toType
            );
        }

        static auto FromInt32(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i32",
                t_natives->Int32,
                t_toType
            );
        }

        static auto FromInt64(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i64",
                t_natives->Int64,
                t_toType
            );
        }

        static auto FromUInt8(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u8",
                t_natives->UInt8,
                t_toType
            );
        }

        static auto FromUInt16(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u16",
                t_natives->UInt16,
                t_toType
            );
        }

        static auto FromUInt32(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u32",
                t_natives->UInt32,
                t_toType
            );
        }

        static auto FromUInt64(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u64",
                t_natives->UInt64,
                t_toType
            );
        }

        static auto FromInt(
            const NativeType& t_toType,
            const Natives* const t_natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_int",
                t_natives->Int,
                t_toType
            );
        }

        static auto UnaryPlus(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryPlus,
                [&](Emitter& t_emitter)
                {
                    t_emitter.GetBlockBuilder().Builder.CreateRet(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType())
                    );
                }
            };
        }

        static auto UnaryNegation(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryNegation,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFMul(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        llvm::ConstantFP::get(t_selfType.GetIRType(), -1)
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Multiplication(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Multiplication,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFMul(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Division(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Division,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFDiv(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );
            
                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Remainder(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Remainder,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFRem(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Addition(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Addition,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFAdd(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Subtraction(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Subtraction,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFSub(
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThan(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OLT,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThan(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OGT,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThanEquals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OLE,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThanEquals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OGE,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Equals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Equals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OEQ,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto NotEquals(
            const NativeType& t_selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::NotEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_ONE,
                        t_emitter.EmitLoadArg(0, t_selfType.GetIRType()),
                        t_emitter.EmitLoadArg(1, t_selfType.GetIRType())
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }
    }

    Natives::Natives(
        const Compilation* const t_compilation
    ) : Int8
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Int8" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt8Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Int16
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Int16" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt16Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Int32
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Int32" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Int64
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Int64" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt64Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        UInt8
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "UInt8" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt8Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        UInt16
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "UInt16" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt16Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        UInt32
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "UInt32" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        UInt64
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "UInt64" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt64Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Int
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Int" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Float32
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Float32" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getFloatTy(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Float64
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Float64" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getDoubleTy(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Bool
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Bool" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt1Ty(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Void
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Void" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getVoidTy(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Unsized,
            NativeCopyabilityKind::NonTrivial,
        },
        String
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "String" } },
                SymbolNameResolutionScope::Global,
            },
            {},
            TypeSizeKind::Sized,
            NativeCopyabilityKind::NonTrivial,
        },

        Pointer
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Pointer" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation]() -> llvm::Type*
            {
                return llvm::Type::getInt8PtrTy(*t_compilation->LLVMContext);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Reference
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "Reference" } },
                SymbolNameResolutionScope::Global,
            }
        },
        StrongPointer
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "rc" }, { "StrongPointer" } },
                SymbolNameResolutionScope::Global,
            }
        },
        WeakPointer
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "rc" }, { "WeakPointer" } },
                SymbolNameResolutionScope::Global,
            }
        },

        print_int
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "print_int" } },
                SymbolNameResolutionScope::Global,
            },
            [this, t_compilation](Emitter& t_emitter)
            {
                std::string string{ "%" PRId32 "\n" };

                auto* const charType = llvm::Type::getInt8Ty(
                    *t_compilation->LLVMContext
                );

                std::vector<llvm::Constant*> chars(string.size());
                for (size_t i = 0; i < string.size(); i++)
                {
                    chars.at(i) = llvm::ConstantInt::get(
                        charType, string.at(i)
                    );
                }

                chars.push_back(llvm::ConstantInt::get(charType, 0));

                auto* const stringType = llvm::ArrayType::get(
                    charType,
                    chars.size()
                );

                auto* const globalDeclaration = llvm::cast<llvm::GlobalVariable>(
                    t_emitter.GetModule().getOrInsertGlobal("printf_string_int", stringType)
                );

                globalDeclaration->setInitializer(
                    llvm::ConstantArray::get(stringType, chars)
                );
                globalDeclaration->setConstant(true);
                globalDeclaration->setLinkage(
                    llvm::GlobalValue::LinkageTypes::PrivateLinkage
                );
                globalDeclaration->setUnnamedAddr(
                    llvm::GlobalValue::UnnamedAddr::Global
                );

                auto* const printfFunction =
                    t_emitter.GetC().GetFunctions().GetPrintf();

                std::vector<llvm::Value*> args{};
                args.push_back(llvm::ConstantExpr::getBitCast(
                    globalDeclaration,
                    llvm::PointerType::get(charType, 0)
                ));
                args.push_back(t_emitter.EmitLoadArg(
                    0, 
                    Int.GetIRType()
                ));

                t_emitter.GetBlockBuilder().Builder.CreateCall(
                    printfFunction,
                    args
                );

                t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        },
        print_ptr
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "print_ptr" } },
                SymbolNameResolutionScope::Global,
            },
            [t_compilation](Emitter& t_emitter)
            {
                std::string string{ "0x%" PRIXPTR "\n" };

                auto* const charType = llvm::Type::getInt8Ty(
                    *t_compilation->LLVMContext
                );

                std::vector<llvm::Constant*> chars(string.size());
                for (size_t i = 0; i < string.size(); i++)
                {
                    chars.at(i) = llvm::ConstantInt::get(
                        charType,
                        string.at(i)
                    );
                }

                chars.push_back(llvm::ConstantInt::get(charType, 0));

                auto* const stringType = llvm::ArrayType::get(
                    charType,
                    chars.size()
                );

                auto* const globalDeclaration = llvm::cast<llvm::GlobalVariable>(
                    t_emitter.GetModule().getOrInsertGlobal("printf_string_ptr", stringType)
                );
                globalDeclaration->setInitializer(
                    llvm::ConstantArray::get(stringType, chars)
                );
                globalDeclaration->setConstant(true);
                globalDeclaration->setLinkage(
                    llvm::GlobalValue::LinkageTypes::PrivateLinkage
                );
                globalDeclaration->setUnnamedAddr(
                    llvm::GlobalValue::UnnamedAddr::Global
                );

                auto* const printfFunction =
                    t_emitter.GetC().GetFunctions().GetPrintf();

                std::vector<llvm::Value*> args{};
                args.push_back(llvm::ConstantExpr::getBitCast(
                    globalDeclaration,
                    llvm::PointerType::get(charType, 0)
                ));
                args.push_back(t_emitter.EmitLoadArg(
                    0,
                    llvm::Type::getInt8PtrTy(*t_compilation->LLVMContext)
                ));

                t_emitter.GetBlockBuilder().Builder.CreateCall(
                    printfFunction,
                    args
                );

                t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        },

        alloc
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "mem" }, { "alloc" } },
                SymbolNameResolutionScope::Global,
            },
            [this](Emitter& t_emitter)
            {
                auto* const mallocFunction =
                    t_emitter.GetC().GetFunctions().GetMalloc();

                auto* const intTypeSymbol = Int.GetSymbol();
                auto* const intType = t_emitter.GetIRType(intTypeSymbol);

                auto* const cSizeType = mallocFunction->arg_begin()->getType();

                auto* const sizeValue = t_emitter.EmitLoadArg(0, intType);

                auto* const convertedSizeValue = t_emitter.GetBlockBuilder().Builder.CreateZExtOrTrunc(
                    sizeValue,
                    cSizeType
                );

                auto* const callInst = t_emitter.GetBlockBuilder().Builder.CreateCall(
                    mallocFunction,
                    { convertedSizeValue }
                );

                t_emitter.GetBlockBuilder().Builder.CreateRet(callInst);
            }
        },
        dealloc
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "mem" }, { "dealloc" } },
                SymbolNameResolutionScope::Global,
            },
            [this](Emitter& t_emitter)
            {
                auto* const freeFunction =
                    t_emitter.GetC().GetFunctions().GetFree();

                auto* const pointerType = Pointer.GetIRType();

                auto* const blockValue = t_emitter.EmitLoadArg(
                    0,
                    pointerType
                );

                t_emitter.GetBlockBuilder().Builder.CreateCall(
                    freeFunction,
                    { blockValue }
                );

                t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        },
        copy
        {
            t_compilation,
            SymbolName
            {
                std::vector<SymbolNameSection>{ { "ace" }, { "std" }, { "mem" }, { "copy" } },
                SymbolNameResolutionScope::Global,
            },
            [this](Emitter& t_emitter)
            {
                auto* const memcpyFunction =
                    t_emitter.GetC().GetFunctions().GetMemcpy();

                auto* const pointerType = Pointer.GetIRType();

                auto* const intTypeSymbol = Int.GetSymbol();
                auto* const intType = t_emitter.GetIRType(intTypeSymbol);

                auto* const cSizeType =
                    (memcpyFunction->arg_begin() + 2)->getType();

                auto* const srcValue = t_emitter.EmitLoadArg(0, pointerType);
                auto* const dstValue = t_emitter.EmitLoadArg(1, pointerType);
                auto* const sizeValue = t_emitter.EmitLoadArg(2, intType);

                auto* const convertedSizeValue = t_emitter.GetBlockBuilder().Builder.CreateZExtOrTrunc(
                    sizeValue,
                    cSizeType
                );

                t_emitter.GetBlockBuilder().Builder.CreateCall(
                    memcpyFunction,
                    { dstValue, srcValue, convertedSizeValue }
                );

                t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        },

        i8__from_i16{ I::FromInt16(Int8, this) },
        i8__from_i32{ I::FromInt32(Int8, this) },
        i8__from_i64{ I::FromInt64(Int8, this) },
        i8__from_u8{ I::FromUInt8(Int8, this) },
        i8__from_u16{ I::FromUInt16(Int8, this) },
        i8__from_u32{ I::FromUInt32(Int8, this) },
        i8__from_u64{ I::FromUInt64(Int8, this) },
        i8__from_int{ I::FromInt(Int8, this) },
        i8__from_f32{ I::FromFloat32(Int8, this) },
        i8__from_f64{ I::FromFloat64(Int8, this) },
        i8__unary_plus{ I::UnaryPlus(Int8) },
        i8__unary_negation{ I::UnaryNegation(Int8) },
        i8__one_complement{ I::OneComplement(Int8) },
        i8__multiplication{ I::Multiplication(Int8) },
        i8__division{ I::Division(Int8) },
        i8__remainder{ I::Remainder(Int8) },
        i8__addition{ I::Addition(Int8) },
        i8__subtraction{ I::Subtraction(Int8) },
        i8__right_shift{ I::RightShift(Int8) },
        i8__left_shift{ I::LeftShift(Int8) },
        i8__less_than{ I::LessThan(Int8) },
        i8__greater_than{ I::GreaterThan(Int8) },
        i8__less_than_equals{ I::LessThanEquals(Int8) },
        i8__greater_than_equals{ I::GreaterThanEquals(Int8) },
        i8__equals{ I::Equals(Int8) },
        i8__not_equals{ I::NotEquals(Int8) },
        i8__AND{ I::AND(Int8) },
        i8__XOR{ I::XOR(Int8) },
        i8__OR{ I::OR(Int8) },

        i16__from_i8{ I::FromInt8(Int16, this) },
        i16__from_i32{ I::FromInt32(Int16, this) },
        i16__from_i64{ I::FromInt64(Int16, this) },
        i16__from_u8{ I::FromUInt8(Int16, this) },
        i16__from_u16{ I::FromUInt16(Int16, this) },
        i16__from_u32{ I::FromUInt32(Int16, this) },
        i16__from_u64{ I::FromUInt64(Int16, this) },
        i16__from_int{ I::FromInt(Int16, this) },
        i16__from_f32{ I::FromFloat32(Int16, this) },
        i16__from_f64{ I::FromFloat64(Int16, this) },
        i16__unary_plus{ I::UnaryPlus(Int16) },
        i16__unary_negation{ I::UnaryNegation(Int16) },
        i16__one_complement{ I::OneComplement(Int16) },
        i16__multiplication{ I::Multiplication(Int16) },
        i16__division{ I::Division(Int16) },
        i16__remainder{ I::Remainder(Int16) },
        i16__addition{ I::Addition(Int16) },
        i16__subtraction{ I::Subtraction(Int16) },
        i16__right_shift{ I::RightShift(Int16) },
        i16__left_shift{ I::LeftShift(Int16) },
        i16__less_than{ I::LessThan(Int16) },
        i16__greater_than{ I::GreaterThan(Int16) },
        i16__less_than_equals{ I::LessThanEquals(Int16) },
        i16__greater_than_equals{ I::GreaterThanEquals(Int16) },
        i16__equals{ I::Equals(Int16) },
        i16__not_equals{ I::NotEquals(Int16) },
        i16__AND{ I::AND(Int16) },
        i16__XOR{ I::XOR(Int16) },
        i16__OR{ I::OR(Int16) },

        i32__from_i8{ I::FromInt8(Int32, this) },
        i32__from_i16{ I::FromInt16(Int32, this) },
        i32__from_i64{ I::FromInt64(Int32, this) },
        i32__from_u8{ I::FromUInt8(Int32, this) },
        i32__from_u16{ I::FromUInt16(Int32, this) },
        i32__from_u32{ I::FromUInt32(Int32, this) },
        i32__from_u64{ I::FromUInt64(Int32, this) },
        i32__from_int{ I::FromInt(Int32, this) },
        i32__from_f32{ I::FromFloat32(Int32, this) },
        i32__from_f64{ I::FromFloat64(Int32, this) },
        i32__unary_plus{ I::UnaryPlus(Int32) },
        i32__unary_negation{ I::UnaryNegation(Int32) },
        i32__one_complement{ I::OneComplement(Int32) },
        i32__multiplication{ I::Multiplication(Int32) },
        i32__division{ I::Division(Int32) },
        i32__remainder{ I::Remainder(Int32) },
        i32__addition{ I::Addition(Int32) },
        i32__subtraction{ I::Subtraction(Int32) },
        i32__right_shift{ I::RightShift(Int32) },
        i32__left_shift{ I::LeftShift(Int32) },
        i32__less_than{ I::LessThan(Int32) },
        i32__greater_than{ I::GreaterThan(Int32) },
        i32__less_than_equals{ I::LessThanEquals(Int32) },
        i32__greater_than_equals{ I::GreaterThanEquals(Int32) },
        i32__equals{ I::Equals(Int32) },
        i32__not_equals{ I::NotEquals(Int32) },
        i32__AND{ I::AND(Int32) },
        i32__XOR{ I::XOR(Int32) },
        i32__OR{ I::OR(Int32) },

        i64__from_i8{ I::FromInt8(Int64, this) },
        i64__from_i16{ I::FromInt16(Int64, this) },
        i64__from_i32{ I::FromInt32(Int64, this) },
        i64__from_u8{ I::FromUInt8(Int64, this) },
        i64__from_u16{ I::FromUInt16(Int64, this) },
        i64__from_u32{ I::FromUInt32(Int64, this) },
        i64__from_u64{ I::FromUInt64(Int64, this) },
        i64__from_int{ I::FromInt(Int64, this) },
        i64__from_f32{ I::FromFloat32(Int64, this) },
        i64__from_f64{ I::FromFloat64(Int64, this) },
        i64__unary_plus{ I::UnaryPlus(Int64) },
        i64__unary_negation{ I::UnaryNegation(Int64) },
        i64__one_complement{ I::OneComplement(Int64) },
        i64__multiplication{ I::Multiplication(Int64) },
        i64__division{ I::Division(Int64) },
        i64__remainder{ I::Remainder(Int64) },
        i64__addition{ I::Addition(Int64) },
        i64__subtraction{ I::Subtraction(Int64) },
        i64__right_shift{ I::RightShift(Int64) },
        i64__left_shift{ I::LeftShift(Int64) },
        i64__less_than{ I::LessThan(Int64) },
        i64__greater_than{ I::GreaterThan(Int64) },
        i64__less_than_equals{ I::LessThanEquals(Int64) },
        i64__greater_than_equals{ I::GreaterThanEquals(Int64) },
        i64__equals{ I::Equals(Int64) },
        i64__not_equals{ I::NotEquals(Int64) },
        i64__AND{ I::AND(Int64) },
        i64__XOR{ I::XOR(Int64) },
        i64__OR{ I::OR(Int64) },

        u8__from_i8{ I::FromInt8(UInt8, this) },
        u8__from_i16{ I::FromInt16(UInt8, this) },
        u8__from_i32{ I::FromInt32(UInt8, this) },
        u8__from_i64{ I::FromInt64(UInt8, this) },
        u8__from_u16{ I::FromUInt16(UInt8, this) },
        u8__from_u32{ I::FromUInt32(UInt8, this) },
        u8__from_u64{ I::FromUInt64(UInt8, this) },
        u8__from_int{ I::FromInt(UInt8, this) },
        u8__from_f32{ I::FromFloat32(UInt8, this) },
        u8__from_f64{ I::FromFloat64(UInt8, this) },
        u8__unary_plus{ I::UnaryPlus(UInt8) },
        u8__unary_negation{ I::UnaryNegation(UInt8) },
        u8__one_complement{ I::OneComplement(UInt8) },
        u8__multiplication{ I::Multiplication(UInt8) },
        u8__division{ I::Division(UInt8) },
        u8__remainder{ I::Remainder(UInt8) },
        u8__addition{ I::Addition(UInt8) },
        u8__subtraction{ I::Subtraction(UInt8) },
        u8__right_shift{ I::RightShift(UInt8) },
        u8__left_shift{ I::LeftShift(UInt8) },
        u8__less_than{ I::LessThan(UInt8) },
        u8__greater_than{ I::GreaterThan(UInt8) },
        u8__less_than_equals{ I::LessThanEquals(UInt8) },
        u8__greater_than_equals{ I::GreaterThanEquals(UInt8) },
        u8__equals{ I::Equals(UInt8) },
        u8__not_equals{ I::NotEquals(UInt8) },
        u8__AND{ I::AND(UInt8) },
        u8__XOR{ I::XOR(UInt8) },
        u8__OR{ I::OR(UInt8) },

        u16__from_i8{ I::FromInt8(UInt16, this) },
        u16__from_i16{ I::FromInt16(UInt16, this) },
        u16__from_i32{ I::FromInt32(UInt16, this) },
        u16__from_i64{ I::FromInt64(UInt16, this) },
        u16__from_u8{ I::FromUInt8(UInt16, this) },
        u16__from_u32{ I::FromUInt32(UInt16, this) },
        u16__from_u64{ I::FromUInt64(UInt16, this) },
        u16__from_int{ I::FromInt(UInt16, this) },
        u16__from_f32{ I::FromFloat32(UInt16, this) },
        u16__from_f64{ I::FromFloat64(UInt16, this) },
        u16__unary_plus{ I::UnaryPlus(UInt16) },
        u16__unary_negation{ I::UnaryNegation(UInt16) },
        u16__one_complement{ I::OneComplement(UInt16) },
        u16__multiplication{ I::Multiplication(UInt16) },
        u16__division{ I::Division(UInt16) },
        u16__remainder{ I::Remainder(UInt16) },
        u16__addition{ I::Addition(UInt16) },
        u16__subtraction{ I::Subtraction(UInt16) },
        u16__right_shift{ I::RightShift(UInt16) },
        u16__left_shift{ I::LeftShift(UInt16) },
        u16__less_than{ I::LessThan(UInt16) },
        u16__greater_than{ I::GreaterThan(UInt16) },
        u16__less_than_equals{ I::LessThanEquals(UInt16) },
        u16__greater_than_equals{ I::GreaterThanEquals(UInt16) },
        u16__equals{ I::Equals(UInt16) },
        u16__not_equals{ I::NotEquals(UInt16) },
        u16__AND{ I::AND(UInt16) },
        u16__XOR{ I::XOR(UInt16) },
        u16__OR{ I::OR(UInt16) },

        u32__from_i8{ I::FromInt8(UInt32, this) },
        u32__from_i16{ I::FromInt16(UInt32, this) },
        u32__from_i32{ I::FromInt32(UInt32, this) },
        u32__from_i64{ I::FromInt64(UInt32, this) },
        u32__from_u8{ I::FromUInt8(UInt32, this) },
        u32__from_u16{ I::FromUInt16(UInt32, this) },
        u32__from_u64{ I::FromUInt64(UInt32, this) },
        u32__from_int{ I::FromInt(UInt32, this) },
        u32__from_f32{ I::FromFloat32(UInt32, this) },
        u32__from_f64{ I::FromFloat64(UInt32, this) },
        u32__unary_plus{ I::UnaryPlus(UInt32) },
        u32__unary_negation{ I::UnaryNegation(UInt32) },
        u32__one_complement{ I::OneComplement(UInt32) },
        u32__multiplication{ I::Multiplication(UInt32) },
        u32__division{ I::Division(UInt32) },
        u32__remainder{ I::Remainder(UInt32) },
        u32__addition{ I::Addition(UInt32) },
        u32__subtraction{ I::Subtraction(UInt32) },
        u32__right_shift{ I::RightShift(UInt32) },
        u32__left_shift{ I::LeftShift(UInt32) },
        u32__less_than{ I::LessThan(UInt32) },
        u32__greater_than{ I::GreaterThan(UInt32) },
        u32__less_than_equals{ I::LessThanEquals(UInt32) },
        u32__greater_than_equals{ I::GreaterThanEquals(UInt32) },
        u32__equals{ I::Equals(UInt32) },
        u32__not_equals{ I::NotEquals(UInt32) },
        u32__AND{ I::AND(UInt32) },
        u32__XOR{ I::XOR(UInt32) },
        u32__OR{ I::OR(UInt32) },

        u64__from_i8{ I::FromInt8(UInt64, this) },
        u64__from_i16{ I::FromInt16(UInt64, this) },
        u64__from_i32{ I::FromInt32(UInt64, this) },
        u64__from_i64{ I::FromInt64(UInt64, this) },
        u64__from_u8{ I::FromUInt8(UInt64, this) },
        u64__from_u16{ I::FromUInt16(UInt64, this) },
        u64__from_u32{ I::FromUInt32(UInt64, this) },
        u64__from_int{ I::FromInt(UInt64, this) },
        u64__from_f32{ I::FromFloat32(UInt64, this) },
        u64__from_f64{ I::FromFloat64(UInt64, this) },
        u64__unary_plus{ I::UnaryPlus(UInt64) },
        u64__unary_negation{ I::UnaryNegation(UInt64) },
        u64__one_complement{ I::OneComplement(UInt64) },
        u64__multiplication{ I::Multiplication(UInt64) },
        u64__division{ I::Division(UInt64) },
        u64__remainder{ I::Remainder(UInt64) },
        u64__addition{ I::Addition(UInt64) },
        u64__subtraction{ I::Subtraction(UInt64) },
        u64__right_shift{ I::RightShift(UInt64) },
        u64__left_shift{ I::LeftShift(UInt64) },
        u64__less_than{ I::LessThan(UInt64) },
        u64__greater_than{ I::GreaterThan(UInt64) },
        u64__less_than_equals{ I::LessThanEquals(UInt64) },
        u64__greater_than_equals{ I::GreaterThanEquals(UInt64) },
        u64__equals{ I::Equals(UInt64) },
        u64__not_equals{ I::NotEquals(UInt64) },
        u64__AND{ I::AND(UInt64) },
        u64__XOR{ I::XOR(UInt64) },
        u64__OR{ I::OR(UInt64) },

        int__from_i8{ I::FromInt8(Int, this) },
        int__from_i16{ I::FromInt16(Int, this) },
        int__from_i32{ I::FromInt32(Int, this) },
        int__from_i64{ I::FromInt64(Int, this) },
        int__from_u8{ I::FromUInt8(Int, this) },
        int__from_u16{ I::FromUInt16(Int, this) },
        int__from_u32{ I::FromUInt32(Int, this) },
        int__from_u64{ I::FromUInt64(Int, this) },
        int__from_f32{ I::FromFloat32(Int, this) },
        int__from_f64{ I::FromFloat64(Int, this) },
        int__unary_plus{ I::UnaryPlus(Int) },
        int__unary_negation{ I::UnaryNegation(Int) },
        int__one_complement{ I::OneComplement(Int) },
        int__multiplication{ I::Multiplication(Int) },
        int__division{ I::Division(Int) },
        int__remainder{ I::Remainder(Int) },
        int__addition{ I::Addition(Int) },
        int__subtraction{ I::Subtraction(Int) },
        int__right_shift{ I::RightShift(Int) },
        int__left_shift{ I::LeftShift(Int) },
        int__less_than{ I::LessThan(Int) },
        int__greater_than{ I::GreaterThan(Int) },
        int__less_than_equals{ I::LessThanEquals(Int) },
        int__greater_than_equals{ I::GreaterThanEquals(Int) },
        int__equals{ I::Equals(Int) },
        int__not_equals{ I::NotEquals(Int) },
        int__AND{ I::AND(Int) },
        int__XOR{ I::XOR(Int) },
        int__OR{ I::OR(Int) },

        f32__from_i8{ F::FromInt8(Float32, this) },
        f32__from_i16{ F::FromInt16(Float32, this) },
        f32__from_i32{ F::FromInt32(Float32, this) },
        f32__from_i64{ F::FromInt64(Float32, this) },
        f32__from_u8{ F::FromUInt8(Float32, this) },
        f32__from_u16{ F::FromUInt16(Float32, this) },
        f32__from_u32{ F::FromUInt32(Float32, this) },
        f32__from_u64{ F::FromUInt64(Float32, this) },
        f32__from_int{ F::FromInt(Float32, this) },
        f32__from_f64
        {
            Float32,
            "from_f64",
            [this, t_compilation](Emitter& t_emitter)
            {
                auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFPTrunc(
                    t_emitter.EmitLoadArg(0, Float64.GetIRType()),
                    llvm::Type::getFloatTy(*t_compilation->LLVMContext)
                );

                t_emitter.GetBlockBuilder().Builder.CreateRet(value);
            }
        },
        f32__unary_plus{ F::UnaryPlus(Float32) },
        f32__unary_negation{ F::UnaryNegation(Float32) },
        f32__multiplication{ F::Multiplication(Float32) },
        f32__division{ F::Division(Float32) },
        f32__remainder{ F::Remainder(Float32) },
        f32__addition{ F::Addition(Float32) },
        f32__subtraction{ F::Subtraction(Float32) },
        f32__less_than{ F::LessThan(Float32) },
        f32__greater_than{ F::GreaterThan(Float32) },
        f32__less_than_equals{ F::LessThanEquals(Float32) },
        f32__greater_than_equals{ F::GreaterThanEquals(Float32) },
        f32__equals{ F::Equals(Float32) },
        f32__not_equals{ F::NotEquals(Float32) },

        f64__from_i8{ F::FromInt8(Float64, this) },
        f64__from_i16{ F::FromInt16(Float64, this) },
        f64__from_i32{ F::FromInt32(Float64, this) },
        f64__from_i64{ F::FromInt64(Float64, this) },
        f64__from_u8{ F::FromUInt8(Float64, this) },
        f64__from_u16{ F::FromUInt16(Float64, this) },
        f64__from_u32{ F::FromUInt32(Float64, this) },
        f64__from_u64{ F::FromUInt64(Float64, this) },
        f64__from_int{ F::FromInt(Float64, this) },
        f64__from_f32
        {
            Float64,
            "from_f32",
            [this, t_compilation](Emitter& t_emitter)
            {
                auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFPExt(
                    t_emitter.EmitLoadArg(0, Float32.GetIRType()),
                    llvm::Type::getDoubleTy(*t_compilation->LLVMContext)
                );

                t_emitter.GetBlockBuilder().Builder.CreateRet(value);
            }
        },
        f64__unary_plus{ F::UnaryPlus(Float64) },
        f64__unary_negation{ F::UnaryNegation(Float64) },
        f64__multiplication{ F::Multiplication(Float64) },
        f64__division{ F::Division(Float64) },
        f64__remainder{ F::Remainder(Float64) },
        f64__addition{ F::Addition(Float64) },
        f64__subtraction{ F::Subtraction(Float64) },
        f64__less_than{ F::LessThan(Float64) },
        f64__greater_than{ F::GreaterThan(Float64) },
        f64__less_than_equals{ F::LessThanEquals(Float64) },
        f64__greater_than_equals{ F::GreaterThanEquals(Float64) },
        f64__equals{ F::Equals(Float64) },
        f64__not_equals{ F::NotEquals(Float64) },

        StrongPointer__new
        {
            StrongPointer,
            "new"
        },
        StrongPointer__value
        {
            StrongPointer,
            "value"
        }
    {
    }

    auto Natives::Initialize() -> void
    {
        InitializeCollectionsOfNatives();

        std::for_each(begin(m_Natives), end(m_Natives),
        [](INative* const t_native)
        {
            t_native->Initialize();
        });

        InitializeIRTypeSymbolMap();

        InitializeImplicitFromOperatorMap();
        InitializeExplicitFromOperatorMap();

        InitializeSignedIntTypesSet();
    }

    auto Natives::GetIRTypeSymbolMap() const -> const std::unordered_map<ITypeSymbol*, llvm::Type*>&
    {
        return m_IRTypeSymbolMap;
    }

    auto Natives::GetImplicitFromOperatorMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>&
    {
        return m_ImplicitFromOperatorMap;
    }

    auto Natives::GetExplicitFromOperatorMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>&
    {
        return m_ExplicitFromOperatorMap;
    }

    auto Natives::IsIntTypeSigned(const NativeType& t_intType) const -> bool
    {
        return m_SignedIntTypesSet.contains(&t_intType);
    }

    auto Natives::InitializeCollectionsOfNatives() -> void
    {
        m_Types = 
        {
            &Int8,
            &Int16,
            &Int32,
            &Int64,
        
            &UInt8,
            &UInt16,
            &UInt32,
            &UInt64,

            &Int,
        
            &Float32,
            &Float64,

            &Bool,
            &Void,
            &String,

            &Pointer,
        };

        m_TypeTemplates = 
        {
            &Reference,
            &StrongPointer,
            &WeakPointer,
        };

        m_Functions =
        {
            &print_int,
            &print_ptr,

            &alloc,
            &dealloc,
            &copy,
        };

        m_FunctionTemplates =
        {
        };

        m_AssociatedFunctions =
        {
            &i8__from_i16,
            &i8__from_i32,
            &i8__from_i64,
            &i8__from_u8,
            &i8__from_u16,
            &i8__from_u32,
            &i8__from_u64,
            &i8__from_int,
            &i8__from_f32,
            &i8__from_f64,
            &i8__unary_plus,
            &i8__unary_negation,
            &i8__one_complement,
            &i8__multiplication,
            &i8__division,
            &i8__remainder,
            &i8__addition,
            &i8__subtraction,
            &i8__right_shift,
            &i8__left_shift,
            &i8__less_than,
            &i8__greater_than,
            &i8__less_than_equals,
            &i8__greater_than_equals,
            &i8__equals,
            &i8__not_equals,
            &i8__AND,
            &i8__XOR,
            &i8__OR,

            &i16__from_i8,
            &i16__from_i32,
            &i16__from_i64,
            &i16__from_u8,
            &i16__from_u16,
            &i16__from_u32,
            &i16__from_u64,
            &i16__from_int,
            &i16__from_f32,
            &i16__from_f64,
            &i16__unary_plus,
            &i16__unary_negation,
            &i16__one_complement,
            &i16__multiplication,
            &i16__division,
            &i16__remainder,
            &i16__addition,
            &i16__subtraction,
            &i16__right_shift,
            &i16__left_shift,
            &i16__less_than,
            &i16__greater_than,
            &i16__less_than_equals,
            &i16__greater_than_equals,
            &i16__equals,
            &i16__not_equals,
            &i16__AND,
            &i16__XOR,
            &i16__OR,

            &i32__from_i8,
            &i32__from_i16,
            &i32__from_i64,
            &i32__from_u8,
            &i32__from_u16,
            &i32__from_u32,
            &i32__from_u64,
            &i32__from_int,
            &i32__from_f32,
            &i32__from_f64,
            &i32__unary_plus,
            &i32__unary_negation,
            &i32__one_complement,
            &i32__multiplication,
            &i32__division,
            &i32__remainder,
            &i32__addition,
            &i32__subtraction,
            &i32__right_shift,
            &i32__left_shift,
            &i32__less_than,
            &i32__greater_than,
            &i32__less_than_equals,
            &i32__greater_than_equals,
            &i32__equals,
            &i32__not_equals,
            &i32__AND,
            &i32__XOR,
            &i32__OR,

            &i64__from_i8,
            &i64__from_i16,
            &i64__from_i32,
            &i64__from_u8,
            &i64__from_u16,
            &i64__from_u32,
            &i64__from_u64,
            &i64__from_int,
            &i64__from_f32,
            &i64__from_f64,
            &i64__unary_plus,
            &i64__unary_negation,
            &i64__one_complement,
            &i64__multiplication,
            &i64__division,
            &i64__remainder,
            &i64__addition,
            &i64__subtraction,
            &i64__right_shift,
            &i64__left_shift,
            &i64__less_than,
            &i64__greater_than,
            &i64__less_than_equals,
            &i64__greater_than_equals,
            &i64__equals,
            &i64__not_equals,
            &i64__AND,
            &i64__XOR,
            &i64__OR,

            &u8__from_i8,
            &u8__from_i16,
            &u8__from_i32,
            &u8__from_i64,
            &u8__from_u16,
            &u8__from_u32,
            &u8__from_u64,
            &u8__from_int,
            &u8__from_f32,
            &u8__from_f64,
            &u8__unary_plus,
            &u8__unary_negation,
            &u8__one_complement,
            &u8__multiplication,
            &u8__division,
            &u8__remainder,
            &u8__addition,
            &u8__subtraction,
            &u8__right_shift,
            &u8__left_shift,
            &u8__less_than,
            &u8__greater_than,
            &u8__less_than_equals,
            &u8__greater_than_equals,
            &u8__equals,
            &u8__not_equals,
            &u8__AND,
            &u8__XOR,
            &u8__OR,

            &u16__from_i8,
            &u16__from_i16,
            &u16__from_i32,
            &u16__from_i64,
            &u16__from_u8,
            &u16__from_u32,
            &u16__from_u64,
            &u16__from_int,
            &u16__from_f32,
            &u16__from_f64,
            &u16__unary_plus,
            &u16__unary_negation,
            &u16__one_complement,
            &u16__multiplication,
            &u16__division,
            &u16__remainder,
            &u16__addition,
            &u16__subtraction,
            &u16__right_shift,
            &u16__left_shift,
            &u16__less_than,
            &u16__greater_than,
            &u16__less_than_equals,
            &u16__greater_than_equals,
            &u16__equals,
            &u16__not_equals,
            &u16__AND,
            &u16__XOR,
            &u16__OR,

            &u32__from_i8,
            &u32__from_i16,
            &u32__from_i32,
            &u32__from_i64,
            &u32__from_u8,
            &u32__from_u16,
            &u32__from_u64,
            &u32__from_int,
            &u32__from_f32,
            &u32__from_f64,
            &u32__unary_plus,
            &u32__unary_negation,
            &u32__one_complement,
            &u32__multiplication,
            &u32__division,
            &u32__remainder,
            &u32__addition,
            &u32__subtraction,
            &u32__right_shift,
            &u32__left_shift,
            &u32__less_than,
            &u32__greater_than,
            &u32__less_than_equals,
            &u32__greater_than_equals,
            &u32__equals,
            &u32__not_equals,
            &u32__AND,
            &u32__XOR,
            &u32__OR,

            &u64__from_i8,
            &u64__from_i16,
            &u64__from_i32,
            &u64__from_i64,
            &u64__from_u8,
            &u64__from_u16,
            &u64__from_u32,
            &u64__from_int,
            &u64__from_f32,
            &u64__from_f64,
            &u64__unary_plus,
            &u64__unary_negation,
            &u64__one_complement,
            &u64__multiplication,
            &u64__division,
            &u64__remainder,
            &u64__addition,
            &u64__subtraction,
            &u64__right_shift,
            &u64__left_shift,
            &u64__less_than,
            &u64__greater_than,
            &u64__less_than_equals,
            &u64__greater_than_equals,
            &u64__equals,
            &u64__not_equals,
            &u64__AND,
            &u64__XOR,
            &u64__OR,

            &int__from_i8,
            &int__from_i16,
            &int__from_i32,
            &int__from_i64,
            &int__from_u8,
            &int__from_u16,
            &int__from_u32,
            &int__from_u64,
            &int__from_f32,
            &int__from_f64,
            &int__unary_plus,
            &int__unary_negation,
            &int__one_complement,
            &int__multiplication,
            &int__division,
            &int__remainder,
            &int__addition,
            &int__subtraction,
            &int__right_shift,
            &int__left_shift,
            &int__less_than,
            &int__greater_than,
            &int__less_than_equals,
            &int__greater_than_equals,
            &int__equals,
            &int__not_equals,
            &int__AND,
            &int__XOR,
            &int__OR,

            &f32__from_i8,
            &f32__from_i16,
            &f32__from_i32,
            &f32__from_i64,
            &f32__from_u8,
            &f32__from_u16,
            &f32__from_u32,
            &f32__from_u64,
            &f32__from_int,
            &f32__from_f64,
            &f32__unary_plus,
            &f32__unary_negation,
            &f32__multiplication,
            &f32__division,
            &f32__remainder,
            &f32__addition,
            &f32__subtraction,
            &f32__less_than,
            &f32__greater_than,
            &f32__less_than_equals,
            &f32__greater_than_equals,
            &f32__equals,
            &f32__not_equals,

            &f64__from_i8,
            &f64__from_i16,
            &f64__from_i32,
            &f64__from_i64,
            &f64__from_u8,
            &f64__from_u16,
            &f64__from_u32,
            &f64__from_u64,
            &f64__from_int,
            &f64__from_f32,
            &f64__unary_plus,
            &f64__unary_negation,
            &f64__multiplication,
            &f64__division,
            &f64__remainder,
            &f64__addition,
            &f64__subtraction,
            &f64__less_than,
            &f64__greater_than,
            &f64__less_than_equals,
            &f64__greater_than_equals,
            &f64__equals,
            &f64__not_equals,
        };

        m_AssociatedFunctionTemplates =
        {
            &StrongPointer__new,
            &StrongPointer__value,
        };

        m_Natives.insert(
            end(m_Natives),
            begin(m_Types),
            end  (m_Types)
        );
        m_Natives.insert(
            end(m_Natives),
            begin(m_TypeTemplates),
            end  (m_TypeTemplates)
        );
        m_Natives.insert(
            end(m_Natives),
            begin(m_Functions),
            end  (m_Functions)
        );
        m_Natives.insert(
            end(m_Natives),
            begin(m_FunctionTemplates),
            end  (m_FunctionTemplates)
        );
        m_Natives.insert(
            end(m_Natives),
            begin(m_AssociatedFunctions),
            end  (m_AssociatedFunctions)
        );
        m_Natives.insert(
            end(m_Natives),
            begin(m_AssociatedFunctionTemplates),
            end  (m_AssociatedFunctionTemplates)
        );
    }

    auto Natives::InitializeIRTypeSymbolMap() -> void
    {
        auto& map = m_IRTypeSymbolMap;

        std::for_each(begin(m_Types), end(m_Types),
        [&](const NativeType* const t_type)
        {
            if (!t_type->HasIRType())
                return;

            map[t_type->GetSymbol()] = t_type->GetIRType();
        });
    }

    auto Natives::InitializeImplicitFromOperatorMap() -> void
    {
        auto& map = m_ImplicitFromOperatorMap;

        map[Int8.GetSymbol()] =
        {
        };
        map[Int16.GetSymbol()] =
        {
            { Int8.GetSymbol(), i16__from_i8.GetSymbol() },
            { UInt8.GetSymbol(), i16__from_u8.GetSymbol() },
        };
        map[Int32.GetSymbol()] = 
        {
            { Int8.GetSymbol(), i32__from_i8.GetSymbol() },
            { Int16.GetSymbol(), i32__from_i16.GetSymbol() },
            { UInt8.GetSymbol(), i32__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), i32__from_u16.GetSymbol() },
        };
        map[Int64.GetSymbol()] =
        {
            { Int8.GetSymbol(), i64__from_i8.GetSymbol() },
            { Int16.GetSymbol(), i64__from_i16.GetSymbol() },
            { Int32.GetSymbol(), i64__from_i32.GetSymbol() },
            { UInt8.GetSymbol(), i64__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), i64__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), i64__from_u32.GetSymbol() },
        };

        map[UInt8.GetSymbol()] =
        {
        };
        map[UInt16.GetSymbol()] =
        {
            { UInt8.GetSymbol(), u16__from_u8.GetSymbol() },
        };
        map[UInt32.GetSymbol()] = 
        {
            { UInt8.GetSymbol(), u32__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), u32__from_u16.GetSymbol() },
        };
        map[UInt64.GetSymbol()] =
        {
            { UInt8.GetSymbol(), u64__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), u64__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), u64__from_u32.GetSymbol() },
        };
        map[Float32.GetSymbol()] =
        {
            { Int8.GetSymbol(), f32__from_i8.GetSymbol() },
            { Int16.GetSymbol(), f32__from_i16.GetSymbol() },
            { Int32.GetSymbol(), f32__from_i32.GetSymbol() },
            { Int64.GetSymbol(), f32__from_i64.GetSymbol() },
            { UInt8.GetSymbol(), f32__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), f32__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), f32__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), f32__from_u64.GetSymbol() },
            { Int.GetSymbol(), f32__from_int.GetSymbol() },
        };
        map[Float64.GetSymbol()] =
        {
            { Int8.GetSymbol(), f64__from_i8.GetSymbol() },
            { Int16.GetSymbol(), f64__from_i16.GetSymbol() },
            { Int32.GetSymbol(), f64__from_i32.GetSymbol() },
            { Int64.GetSymbol(), f64__from_i64.GetSymbol() },
            { UInt8.GetSymbol(), f64__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), f64__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), f64__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), f64__from_u64.GetSymbol() },
            { Int.GetSymbol(), f64__from_int.GetSymbol() },
            { Float32.GetSymbol(), f64__from_f32.GetSymbol() },
        };
    }

    auto Natives::InitializeExplicitFromOperatorMap() -> void
    {
        auto& map = m_ExplicitFromOperatorMap;

        map[Int8.GetSymbol()] =
        {
            { Int16.GetSymbol(), i8__from_i16.GetSymbol() },
            { Int32.GetSymbol(), i8__from_i32.GetSymbol() },
            { Int64.GetSymbol(), i8__from_i64.GetSymbol() },
            { UInt8.GetSymbol(), i8__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), i8__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), i8__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), i8__from_u64.GetSymbol() },
            { Int.GetSymbol(), i8__from_int.GetSymbol() },
            { Float32.GetSymbol(), i8__from_f32.GetSymbol() },
            { Float64.GetSymbol(), i8__from_f64.GetSymbol() },
        };
        map[Int16.GetSymbol()] =
        {
            { Int32.GetSymbol(), i16__from_i32.GetSymbol() },
            { Int64.GetSymbol(), i16__from_i64.GetSymbol() },
            { UInt16.GetSymbol(), i16__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), i16__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), i16__from_u64.GetSymbol() },
            { Int.GetSymbol(), i16__from_int.GetSymbol() },
            { Float32.GetSymbol(), i16__from_f32.GetSymbol() },
            { Float64.GetSymbol(), i16__from_f64.GetSymbol() },
        };
        map[Int32.GetSymbol()] =
        {
            { Int64.GetSymbol(), i32__from_i64.GetSymbol() },
            { UInt32.GetSymbol(), i32__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), i32__from_u64.GetSymbol() },
            { Int.GetSymbol(), i32__from_int.GetSymbol() },
            { Float32.GetSymbol(), i32__from_f32.GetSymbol() },
            { Float64.GetSymbol(), i32__from_f64.GetSymbol() },
        };
        map[Int64.GetSymbol()] =
        {
            { UInt64.GetSymbol(), i64__from_u64.GetSymbol() },
            { Int.GetSymbol(), i64__from_int.GetSymbol() },
            { Float32.GetSymbol(), i64__from_f32.GetSymbol() },
            { Float64.GetSymbol(), i64__from_f64.GetSymbol() },
        };

        map[UInt8.GetSymbol()] =
        {
            { Int8.GetSymbol(), u8__from_i8.GetSymbol() },
            { Int16.GetSymbol(), u8__from_i16.GetSymbol() },
            { Int32.GetSymbol(), u8__from_i32.GetSymbol() },
            { Int64.GetSymbol(), u8__from_i64.GetSymbol() },
            { UInt16.GetSymbol(), u8__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), u8__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), u8__from_u64.GetSymbol() },
            { Int.GetSymbol(), u8__from_int.GetSymbol() },
            { Float32.GetSymbol(), u8__from_f32.GetSymbol() },
            { Float64.GetSymbol(), u8__from_f64.GetSymbol() },
        };
        map[UInt16.GetSymbol()] =
        {
            { Int8.GetSymbol(), u16__from_i8.GetSymbol() },
            { Int16.GetSymbol(), u16__from_i16.GetSymbol() },
            { Int32.GetSymbol(), u16__from_i32.GetSymbol() },
            { Int64.GetSymbol(), u16__from_i64.GetSymbol() },
            { UInt32.GetSymbol(), u16__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), u16__from_u64.GetSymbol() },
            { Int.GetSymbol(), u16__from_int.GetSymbol() },
            { Float32.GetSymbol(), u16__from_f32.GetSymbol() },
            { Float64.GetSymbol(), u16__from_f64.GetSymbol() },
        };
        map[UInt32.GetSymbol()] =
        {
            { Int8.GetSymbol(), u32__from_i8.GetSymbol() },
            { Int16.GetSymbol(), u32__from_i16.GetSymbol() },
            { Int32.GetSymbol(), u32__from_i32.GetSymbol() },
            { Int64.GetSymbol(), u32__from_i64.GetSymbol() },
            { UInt64.GetSymbol(), u32__from_u64.GetSymbol() },
            { Int.GetSymbol(), u32__from_int.GetSymbol() },
            { Float32.GetSymbol(), u32__from_f32.GetSymbol() },
            { Float64.GetSymbol(), u32__from_f64.GetSymbol() },
        };
        map[UInt64.GetSymbol()] =
        {
            { Int8.GetSymbol(), u64__from_i8.GetSymbol() },
            { Int16.GetSymbol(), u64__from_i16.GetSymbol() },
            { Int32.GetSymbol(), u64__from_i32.GetSymbol() },
            { Int64.GetSymbol(), u64__from_i64.GetSymbol() },
            { Int.GetSymbol(), u64__from_int.GetSymbol() },
            { Float32.GetSymbol(), u64__from_f32.GetSymbol() },
            { Float64.GetSymbol(), u64__from_f64.GetSymbol() },
        };

        map[Int.GetSymbol()] =
        {
            { Int8.GetSymbol(), int__from_i8.GetSymbol() },
            { Int16.GetSymbol(), int__from_i16.GetSymbol() },
            { Int32.GetSymbol(), int__from_i32.GetSymbol() },
            { Int64.GetSymbol(), int__from_i64.GetSymbol() },
            { UInt8.GetSymbol(), int__from_u8.GetSymbol() },
            { UInt16.GetSymbol(), int__from_u16.GetSymbol() },
            { UInt32.GetSymbol(), int__from_u32.GetSymbol() },
            { UInt64.GetSymbol(), int__from_u64.GetSymbol() },
            { Float32.GetSymbol(), int__from_f32.GetSymbol() },
            { Float64.GetSymbol(), int__from_f64.GetSymbol() },
        };

        map[Float32.GetSymbol()] =
        {
            { Float64.GetSymbol(), f32__from_f64.GetSymbol() },
        };
        map[Float64.GetSymbol()] =
        {
        };
    }

    auto Natives::InitializeSignedIntTypesSet() -> void
    {
        m_SignedIntTypesSet = 
        {
            &Int8,
            &Int16,
            &Int32,
            &Int64,
            &Int,
        };
    }
}
