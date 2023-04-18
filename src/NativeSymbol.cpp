#include "NativeSymbol.hpp"

#include <vector>
#include <optional>
#include <algorithm>
#include <set>

#include "Error.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Template/Type.hpp"
#include "Symbol/Template/Function.hpp"
#include "Emittable.hpp"
#include "Scope.hpp"
#include "Emitter.hpp"
#include "SpecialIdentifier.hpp"
#include "Error.hpp"

namespace Ace::NativeSymbol
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

    auto Type::Initialize() -> Expected<void>
    {
        ACE_TRY(symbol, Scope::GetRoot()->ResolveStaticSymbol<Symbol::Type::IBase>(m_Name));

        if (m_IRTypeGetter.has_value())
        {
            symbol->SetAsNativeSized();
        }

        if (m_IsTriviallyCopyable)
        {
            symbol->SetAsTriviallyCopyable();
        }

        m_Symbol = symbol;
        return ExpectedVoid;
    }

    auto TypeTemplate::Initialize() -> Expected<void> 
    {
        auto name = m_Name;
        name.Sections.back().Name = SpecialIdentifier::CreateTemplate(name.Sections.back().Name);

        ACE_TRY(symbol, Scope::GetRoot()->ResolveStaticSymbol<Symbol::Template::Type>(name));
        m_Symbol = symbol;
        return ExpectedVoid;
    }

    auto Function::Initialize() -> Expected<void>
    {
        ACE_TRY(symbol, Scope::GetRoot()->ResolveStaticSymbol<Symbol::Function>(m_Name));

        const auto emittableBody = std::make_shared<FunctionEmittableBody>(m_BodyEmitter);
        symbol->BindBody(emittableBody);
        symbol->SetAsNative();

        m_Symbol = symbol;
        return ExpectedVoid;
    }

    auto FunctionTemplate::Initialize() -> Expected<void>
    {
        ACE_TRY(symbol, Scope::GetRoot()->ResolveStaticSymbol<Symbol::Template::Function>(m_Name));
        m_Symbol = symbol;
        return ExpectedVoid;
    }

    auto AssociatedFunction::Initialize() -> Expected<void>
    {
        auto name = m_Type.GetFullyQualifiedName();
        name.Sections.emplace_back(m_Name);

        ACE_TRY(symbol, Scope::GetRoot()->ResolveStaticSymbol<Symbol::Function>(name));

        const auto emittableBody = std::make_shared<FunctionEmittableBody>(m_BodyEmitter);
        symbol->BindBody(emittableBody);
        symbol->SetAsNative();

        m_Symbol = symbol;
        return ExpectedVoid;
    }

    auto AssociatedFunctionTemplate::Initialize() -> Expected<void>
    {
        auto name = m_Type.GetFullyQualifiedName();
        name.Sections.emplace_back(m_Name);

        ACE_TRY(symbol, Scope::GetRoot()->ResolveStaticSymbol<Symbol::Template::Function>(name));

        m_Symbol = symbol;
        return ExpectedVoid;
    }

    static std::set<const Type*> SignedIntTypesSet
    {
        &Int8,
        &Int16,
        &Int32,
        &Int64,
        &Int,
    };

    static auto IsIntTypeSigned(const Type& t_intType) -> bool
    {
        return SignedIntTypesSet.contains(&t_intType);
    }

    namespace I
    {        
        static AssociatedFunction FromIntImpl(const char* const t_name, const Type& t_fromType, const Type& t_toType)
        {
            return
            {
                t_toType,
                t_name,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (IsIntTypeSigned(t_fromType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSExtOrTrunc(
                                t_emitter.EmitLoadArgument(0, t_fromType.GetIRType(t_emitter)),
                                t_toType.GetIRType(t_emitter)
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateZExtOrTrunc(
                                t_emitter.EmitLoadArgument(0, t_fromType.GetIRType(t_emitter)),
                                t_toType.GetIRType(t_emitter)
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction FromInt8(const Type& t_toType)
        {
            return FromIntImpl("from_i8", Int8, t_toType);
        }

        static AssociatedFunction FromInt16(const Type& t_toType)
        {
            return FromIntImpl("from_i16", Int16, t_toType);
        }

        static AssociatedFunction FromInt32(const Type& t_toType)
        {
            return FromIntImpl("from_i32", Int32, t_toType);
        }

        static AssociatedFunction FromInt64(const Type& t_toType)
        {
            return FromIntImpl("from_i64", Int64, t_toType);
        }

        static AssociatedFunction FromUInt8(const Type& t_toType)
        {
            return FromIntImpl("from_u8", UInt8, t_toType);
        }

        static AssociatedFunction FromUInt16(const Type& t_toType)
        {
            return FromIntImpl("from_u16", UInt16, t_toType);
        }

        static AssociatedFunction FromUInt32(const Type& t_toType)
        {
            return FromIntImpl("from_u32", UInt32, t_toType);
        }

        static AssociatedFunction FromUInt64(const Type& t_toType)
        {
            return FromIntImpl("from_u64", UInt64, t_toType);
        }

        static AssociatedFunction FromInt(const Type& t_toType)
        {
            return FromIntImpl("from_int", Int, t_toType);
        }

        static AssociatedFunction FromFloat(const char* const t_name, const Type& t_fromType, const Type& t_toType)
        {
            return
            {
                t_toType,
                t_name,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (IsIntTypeSigned(t_toType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateFPToSI(
                                t_emitter.EmitLoadArgument(0, t_fromType.GetIRType(t_emitter)),
                                t_toType.GetIRType(t_emitter)
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateFPToUI(
                                t_emitter.EmitLoadArgument(0, t_fromType.GetIRType(t_emitter)),
                                t_toType.GetIRType(t_emitter)
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction FromFloat32(const Type& t_toType)
        {
            return FromFloat("from_f32", Float32, t_toType);
        }

        static AssociatedFunction FromFloat64(const Type& t_toType)
        {
            return FromFloat("from_f64", Float64, t_toType);
        }

        static AssociatedFunction Division(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Division,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (IsIntTypeSigned(t_selfType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSDiv(
                                t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                                t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateUDiv(
                                t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                                t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Remainder(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Remainder,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (IsIntTypeSigned(t_selfType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSRem(
                                t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                                t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateURem(
                                t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                                t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction UnaryPlus(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryPlus,
                [&](Emitter& t_emitter)
                {
                    t_emitter.GetBlockBuilder().Builder.CreateRet(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter))
                    );
                }
            };
        }

        static AssociatedFunction UnaryNegation(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryNegation,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateMul(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        llvm::ConstantInt::get(t_selfType.GetIRType(t_emitter), -1)
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };

        }

        static AssociatedFunction OneComplement(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::OneComplement,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateXor(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        llvm::ConstantInt::get(t_selfType.GetIRType(t_emitter), -1)
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Multiplication(const Type& t_selfType)
        {
            return 
            {
                t_selfType,
                SpecialIdentifier::Operator::Multiplication,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateMul(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Addition(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Addition,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateAdd(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Subtraction(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Subtraction,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateSub(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction RightShift(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::RightShift,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateAShr(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        };

        static AssociatedFunction LeftShift(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LeftShift,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateShl(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction LessThan(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLT,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction GreaterThan(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGT,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction LessThanEquals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLE,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction GreaterThanEquals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGE,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Equals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Equals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_EQ,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction NotEquals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::NotEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_NE,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction AND(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::AND,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateAnd(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction XOR(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::XOR,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateXor(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction OR(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::OR,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateOr(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }
    }

    namespace F
    {
        static AssociatedFunction FromIntImpl(const char* const t_name, const Type& t_fromType, const Type& t_toType)
        {
            return
            {
                t_toType,
                t_name,
                [&](Emitter& t_emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (IsIntTypeSigned(t_fromType))
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateSIToFP(
                                t_emitter.EmitLoadArgument(0, t_fromType.GetIRType(t_emitter)),
                                t_toType.GetIRType(t_emitter)
                            );
                        }
                        else
                        {
                            return t_emitter.GetBlockBuilder().Builder.CreateUIToFP(
                                t_emitter.EmitLoadArgument(0, t_fromType.GetIRType(t_emitter)),
                                t_toType.GetIRType(t_emitter)
                            );
                        }
                    }();

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction FromInt8(const Type& t_toType)
        {
            return FromIntImpl("from_i8", Int8, t_toType);
        }

        static AssociatedFunction FromInt16(const Type& t_toType)
        {
            return FromIntImpl("from_i16", Int16, t_toType);
        }

        static AssociatedFunction FromInt32(const Type& t_toType)
        {
            return FromIntImpl("from_i32", Int32, t_toType);
        }

        static AssociatedFunction FromInt64(const Type& t_toType)
        {
            return FromIntImpl("from_i64", Int64, t_toType);
        }

        static AssociatedFunction FromUInt8(const Type& t_toType)
        {
            return FromIntImpl("from_u8", UInt8, t_toType);
        }

        static AssociatedFunction FromUInt16(const Type& t_toType)
        {
            return FromIntImpl("from_u16", UInt16, t_toType);
        }

        static AssociatedFunction FromUInt32(const Type& t_toType)
        {
            return FromIntImpl("from_u32", UInt32, t_toType);
        }

        static AssociatedFunction FromUInt64(const Type& t_toType)
        {
            return FromIntImpl("from_u64", UInt64, t_toType);
        }

        static AssociatedFunction FromInt(const Type& t_toType)
        {
            return FromIntImpl("from_int", Int, t_toType);
        }

        static AssociatedFunction UnaryPlus(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryPlus,
                [&](Emitter& t_emitter)
                {
                    t_emitter.GetBlockBuilder().Builder.CreateRet(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter))
                    );
                }
            };
        }

        static AssociatedFunction UnaryNegation(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::UnaryNegation,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFMul(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        llvm::ConstantFP::get(t_selfType.GetIRType(t_emitter), -1)
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Multiplication(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Multiplication,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFMul(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Division(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Division,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFDiv(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );
            
                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Remainder(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Remainder,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFRem(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Addition(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Addition,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFAdd(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Subtraction(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Subtraction,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFSub(
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction LessThan(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OLT,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction GreaterThan(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThan,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OGT,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction LessThanEquals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::LessThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OLE,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction GreaterThanEquals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::GreaterThanEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OGE,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction Equals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::Equals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OEQ,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static AssociatedFunction NotEquals(const Type& t_selfType)
        {
            return
            {
                t_selfType,
                SpecialIdentifier::Operator::NotEquals,
                [&](Emitter& t_emitter)
                {
                    auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_ONE,
                        t_emitter.EmitLoadArgument(0, t_selfType.GetIRType(t_emitter)),
                        t_emitter.EmitLoadArgument(1, t_selfType.GetIRType(t_emitter))
                    );

                    t_emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }
    }

    Type Int8
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Int8" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt8Ty(t_emitter.GetContext());
        },
        true,
    };

    Type Int16
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Int16" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt16Ty(t_emitter.GetContext());
        },
        true,
    };

    Type Int32
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Int32" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt32Ty(t_emitter.GetContext());
        },
        true,
    };

    Type Int64
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Int64" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt64Ty(t_emitter.GetContext());
        },
        true,
    };

    Type UInt8
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "UInt8" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt8Ty(t_emitter.GetContext());
        },
        true,
    };

    Type UInt16
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "UInt16" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt16Ty(t_emitter.GetContext());
        },
        true,
    };

    Type UInt32
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "UInt32" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt32Ty(t_emitter.GetContext());
        },
        true,
    };

    Type UInt64
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "UInt64" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt64Ty(t_emitter.GetContext());
        },
        true,
    };

    Type Int
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Int" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt32Ty(t_emitter.GetContext());
        },
        true,
    };

    Type Float32
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Float32" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getFloatTy(t_emitter.GetContext());
        },
        true,
    };

    Type Float64
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Float64" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getDoubleTy(t_emitter.GetContext());
        },
        true,
    };

    Type Bool
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Bool" } }, true},
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt1Ty(t_emitter.GetContext());
        },
        true,
    };

    Type Void
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Void" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getVoidTy(t_emitter.GetContext());
        },
        false,
    };

    Type String
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "String" } }, true },
        {},
        false,
    };

    Type Pointer
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Pointer" } }, true },
        [](Emitter& t_emitter) -> llvm::Type*
        {
            return llvm::Type::getInt8PtrTy(t_emitter.GetContext());
        },
        true
    };

    TypeTemplate Reference
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "Reference" } }, true }
    };

    TypeTemplate StrongPointer
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "rc" }, { "StrongPointer" } }, true }
    };

    TypeTemplate WeakPointer
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "rc" }, { "WeakPointer" } }, true }
    };

    Function print_int
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "print_int" } }, true },
        [](Emitter& t_emitter)
        {
            std::string string{ "%" PRId32 "\n" };

            auto* const charType = llvm::Type::getInt8Ty(t_emitter.GetContext());

            std::vector<llvm::Constant*> chars(string.size());
            for (size_t i = 0; i < string.size(); i++)
                chars[i] = llvm::ConstantInt::get(charType, string[i]);

            chars.push_back(llvm::ConstantInt::get(charType, 0));

            auto* const stringType = llvm::ArrayType::get(charType, chars.size());

            auto* const globalDeclaration = llvm::cast<llvm::GlobalVariable>(t_emitter.GetModule().getOrInsertGlobal("printf_string_int", stringType));
            globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, chars));
            globalDeclaration->setConstant(true);
            globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
            globalDeclaration->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

            auto* const printfFunction = t_emitter.GetC().GetFunctions().GetPrintf();

            std::vector<llvm::Value*> arguments{};
            arguments.push_back(llvm::ConstantExpr::getBitCast(globalDeclaration, llvm::PointerType::get(charType, 0)));
            arguments.push_back(t_emitter.EmitLoadArgument(0, NativeSymbol::Int.GetIRType(t_emitter)));

            t_emitter.GetBlockBuilder().Builder.CreateCall(
                printfFunction,
                arguments
            );

            t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
        }
    };

    Function print_ptr
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "print_ptr" } }, true },
        [](Emitter& t_emitter)
        {
            std::string string{ "0x%" PRIXPTR "\n" };

            auto* const charType = llvm::Type::getInt8Ty(t_emitter.GetContext());

            std::vector<llvm::Constant*> chars(string.size());
            for (size_t i = 0; i < string.size(); i++)
            {
                chars[i] = llvm::ConstantInt::get(charType, string[i]);
            }

            chars.push_back(llvm::ConstantInt::get(charType, 0));

            auto* const stringType = llvm::ArrayType::get(charType, chars.size());

            auto* const globalDeclaration = llvm::cast<llvm::GlobalVariable>(t_emitter.GetModule().getOrInsertGlobal("printf_string_ptr", stringType));
            globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, chars));
            globalDeclaration->setConstant(true);
            globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
            globalDeclaration->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

            auto* const printfFunction = t_emitter.GetC().GetFunctions().GetPrintf();

            std::vector<llvm::Value*> arguments{};
            arguments.push_back(llvm::ConstantExpr::getBitCast(globalDeclaration, llvm::PointerType::get(charType, 0)));
            arguments.push_back(t_emitter.EmitLoadArgument(0, llvm::Type::getInt8PtrTy(t_emitter.GetContext())));

            t_emitter.GetBlockBuilder().Builder.CreateCall(
                printfFunction,
                arguments
            );

            t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
        }
    };

    Function alloc
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "mem" }, { "alloc" } }, true },
        [](Emitter& t_emitter)
        {
            auto* const mallocFunction = t_emitter.GetC().GetFunctions().GetMalloc();

            auto* const intTypeSymbol = Int.GetSymbol();
            auto* const intType = t_emitter.GetIRType(intTypeSymbol);

            auto* const cSizeType = mallocFunction->arg_begin()->getType();

            auto* const sizeValue = t_emitter.EmitLoadArgument(0, intType);

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
    };

    Function dealloc
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "mem" }, { "dealloc" } }, true },
        [](Emitter& t_emitter)
        {
            auto* const freeFunction = t_emitter.GetC().GetFunctions().GetFree();

            auto* const pointerType = Pointer.GetIRType(t_emitter);

            auto* const blockValue = t_emitter.EmitLoadArgument(0, pointerType);

            t_emitter.GetBlockBuilder().Builder.CreateCall(
                freeFunction,
                { blockValue }
            );

            t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
        }
    };

    Function copy
    {
        Name::Symbol::Full{ std::vector<Name::Symbol::Section>{ { "ace" }, { "std" }, { "mem" }, { "copy" } }, true },
        [](Emitter& t_emitter)
        {
            auto* const memcpyFunction = t_emitter.GetC().GetFunctions().GetMemcpy();

            auto* const pointerType = Pointer.GetIRType(t_emitter);

            auto* const intTypeSymbol = Int.GetSymbol();
            auto* const intType = t_emitter.GetIRType(intTypeSymbol);

            auto* const cSizeType = (memcpyFunction->arg_begin() + 2)->getType();

            auto* const srcValue = t_emitter.EmitLoadArgument(0, pointerType);
            auto* const dstValue = t_emitter.EmitLoadArgument(1, pointerType);
            auto* const sizeValue = t_emitter.EmitLoadArgument(2, intType);

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
    };

    AssociatedFunction i8__from_i16 = I::FromInt16(Int8);
    AssociatedFunction i8__from_i32 = I::FromInt32(Int8);
    AssociatedFunction i8__from_i64 = I::FromInt64(Int8);
    AssociatedFunction i8__from_u8 = I::FromUInt8(Int8);
    AssociatedFunction i8__from_u16 = I::FromUInt16(Int8);
    AssociatedFunction i8__from_u32 = I::FromUInt32(Int8);
    AssociatedFunction i8__from_u64 = I::FromUInt64(Int8);
    AssociatedFunction i8__from_int = I::FromInt(Int8);
    AssociatedFunction i8__from_f32 = I::FromFloat32(Int8);
    AssociatedFunction i8__from_f64 = I::FromFloat64(Int8);
    AssociatedFunction i8__unary_plus = I::UnaryPlus(Int8);
    AssociatedFunction i8__unary_negation = I::UnaryNegation(Int8);
    AssociatedFunction i8__one_complement = I::OneComplement(Int8);
    AssociatedFunction i8__multiplication = I::Multiplication(Int8);
    AssociatedFunction i8__division = I::Division(Int8);
    AssociatedFunction i8__remainder = I::Remainder(Int8);
    AssociatedFunction i8__addition = I::Addition(Int8);
    AssociatedFunction i8__subtraction = I::Subtraction(Int8);
    AssociatedFunction i8__right_shift = I::RightShift(Int8);
    AssociatedFunction i8__left_shift = I::LeftShift(Int8);
    AssociatedFunction i8__less_than = I::LessThan(Int8);
    AssociatedFunction i8__greater_than = I::GreaterThan(Int8);
    AssociatedFunction i8__less_than_equals = I::LessThanEquals(Int8);
    AssociatedFunction i8__greater_than_equals = I::GreaterThanEquals(Int8);
    AssociatedFunction i8__equals = I::Equals(Int8);
    AssociatedFunction i8__not_equals = I::NotEquals(Int8);
    AssociatedFunction i8__AND = I::AND(Int8);
    AssociatedFunction i8__XOR = I::XOR(Int8);
    AssociatedFunction i8__OR = I::OR(Int8);

    AssociatedFunction i16__from_i8 = I::FromInt8(Int16);
    AssociatedFunction i16__from_i32 = I::FromInt32(Int16);
    AssociatedFunction i16__from_i64 = I::FromInt64(Int16);
    AssociatedFunction i16__from_u8 = I::FromUInt8(Int16);
    AssociatedFunction i16__from_u16 = I::FromUInt16(Int16);
    AssociatedFunction i16__from_u32 = I::FromUInt32(Int16);
    AssociatedFunction i16__from_u64 = I::FromUInt64(Int16);
    AssociatedFunction i16__from_int = I::FromInt(Int16);
    AssociatedFunction i16__from_f32 = I::FromFloat32(Int16);
    AssociatedFunction i16__from_f64 = I::FromFloat64(Int16);
    AssociatedFunction i16__unary_plus = I::UnaryPlus(Int16);
    AssociatedFunction i16__unary_negation = I::UnaryNegation(Int16);
    AssociatedFunction i16__one_complement = I::OneComplement(Int16);
    AssociatedFunction i16__multiplication = I::Multiplication(Int16);
    AssociatedFunction i16__division = I::Division(Int16);
    AssociatedFunction i16__remainder = I::Remainder(Int16);
    AssociatedFunction i16__addition = I::Addition(Int16);
    AssociatedFunction i16__subtraction = I::Subtraction(Int16);
    AssociatedFunction i16__right_shift = I::RightShift(Int16);
    AssociatedFunction i16__left_shift = I::LeftShift(Int16);
    AssociatedFunction i16__less_than = I::LessThan(Int16);
    AssociatedFunction i16__greater_than = I::GreaterThan(Int16);
    AssociatedFunction i16__less_than_equals = I::LessThanEquals(Int16);
    AssociatedFunction i16__greater_than_equals = I::GreaterThanEquals(Int16);
    AssociatedFunction i16__equals = I::Equals(Int16);
    AssociatedFunction i16__not_equals = I::NotEquals(Int16);
    AssociatedFunction i16__AND = I::AND(Int16);
    AssociatedFunction i16__XOR = I::XOR(Int16);
    AssociatedFunction i16__OR = I::OR(Int16);

    AssociatedFunction i32__from_i8 = I::FromInt8(Int32);
    AssociatedFunction i32__from_i16 = I::FromInt16(Int32);
    AssociatedFunction i32__from_i64 = I::FromInt64(Int32);
    AssociatedFunction i32__from_u8 = I::FromUInt8(Int32);
    AssociatedFunction i32__from_u16 = I::FromUInt16(Int32);
    AssociatedFunction i32__from_u32 = I::FromUInt32(Int32);
    AssociatedFunction i32__from_u64 = I::FromUInt64(Int32);
    AssociatedFunction i32__from_int = I::FromInt(Int32);
    AssociatedFunction i32__from_f32 = I::FromFloat32(Int32);
    AssociatedFunction i32__from_f64 = I::FromFloat64(Int32);
    AssociatedFunction i32__unary_plus = I::UnaryPlus(Int32);
    AssociatedFunction i32__unary_negation = I::UnaryNegation(Int32);
    AssociatedFunction i32__one_complement = I::OneComplement(Int32);
    AssociatedFunction i32__multiplication = I::Multiplication(Int32);
    AssociatedFunction i32__division = I::Division(Int32);
    AssociatedFunction i32__remainder = I::Remainder(Int32);
    AssociatedFunction i32__addition = I::Addition(Int32);
    AssociatedFunction i32__subtraction = I::Subtraction(Int32);
    AssociatedFunction i32__right_shift = I::RightShift(Int32);
    AssociatedFunction i32__left_shift = I::LeftShift(Int32);
    AssociatedFunction i32__less_than = I::LessThan(Int32);
    AssociatedFunction i32__greater_than = I::GreaterThan(Int32);
    AssociatedFunction i32__less_than_equals = I::LessThanEquals(Int32);
    AssociatedFunction i32__greater_than_equals = I::GreaterThanEquals(Int32);
    AssociatedFunction i32__equals = I::Equals(Int32);
    AssociatedFunction i32__not_equals = I::NotEquals(Int32);
    AssociatedFunction i32__AND = I::AND(Int32);
    AssociatedFunction i32__XOR = I::XOR(Int32);
    AssociatedFunction i32__OR = I::OR(Int32);

    AssociatedFunction i64__from_i8 = I::FromInt8(Int64);
    AssociatedFunction i64__from_i16 = I::FromInt16(Int64);
    AssociatedFunction i64__from_i32 = I::FromInt32(Int64);
    AssociatedFunction i64__from_u8 = I::FromUInt8(Int64);
    AssociatedFunction i64__from_u16 = I::FromUInt16(Int64);
    AssociatedFunction i64__from_u32 = I::FromUInt32(Int64);
    AssociatedFunction i64__from_u64 = I::FromUInt64(Int64);
    AssociatedFunction i64__from_int = I::FromInt(Int64);
    AssociatedFunction i64__from_f32 = I::FromFloat32(Int64);
    AssociatedFunction i64__from_f64 = I::FromFloat64(Int64);
    AssociatedFunction i64__unary_plus = I::UnaryPlus(Int64);
    AssociatedFunction i64__unary_negation = I::UnaryNegation(Int64);
    AssociatedFunction i64__one_complement = I::OneComplement(Int64);
    AssociatedFunction i64__multiplication = I::Multiplication(Int64);
    AssociatedFunction i64__division = I::Division(Int64);
    AssociatedFunction i64__remainder = I::Remainder(Int64);
    AssociatedFunction i64__addition = I::Addition(Int64);
    AssociatedFunction i64__subtraction = I::Subtraction(Int64);
    AssociatedFunction i64__right_shift = I::RightShift(Int64);
    AssociatedFunction i64__left_shift = I::LeftShift(Int64);
    AssociatedFunction i64__less_than = I::LessThan(Int64);
    AssociatedFunction i64__greater_than = I::GreaterThan(Int64);
    AssociatedFunction i64__less_than_equals = I::LessThanEquals(Int64);
    AssociatedFunction i64__greater_than_equals = I::GreaterThanEquals(Int64);
    AssociatedFunction i64__equals = I::Equals(Int64);
    AssociatedFunction i64__not_equals = I::NotEquals(Int64);
    AssociatedFunction i64__AND = I::AND(Int64);
    AssociatedFunction i64__XOR = I::XOR(Int64);
    AssociatedFunction i64__OR = I::OR(Int64);

    AssociatedFunction u8__from_i8 = I::FromInt8(UInt8);
    AssociatedFunction u8__from_i16 = I::FromInt16(UInt8);
    AssociatedFunction u8__from_i32 = I::FromInt32(UInt8);
    AssociatedFunction u8__from_i64 = I::FromInt64(UInt8);
    AssociatedFunction u8__from_u16 = I::FromUInt16(UInt8);
    AssociatedFunction u8__from_u32 = I::FromUInt32(UInt8);
    AssociatedFunction u8__from_u64 = I::FromUInt64(UInt8);
    AssociatedFunction u8__from_int = I::FromInt(UInt8);
    AssociatedFunction u8__from_f32 = I::FromFloat32(UInt8);
    AssociatedFunction u8__from_f64 = I::FromFloat64(UInt8);
    AssociatedFunction u8__unary_plus = I::UnaryPlus(UInt8);
    AssociatedFunction u8__unary_negation = I::UnaryNegation(UInt8);
    AssociatedFunction u8__one_complement = I::OneComplement(UInt8);
    AssociatedFunction u8__multiplication = I::Multiplication(UInt8);
    AssociatedFunction u8__division = I::Division(UInt8);
    AssociatedFunction u8__remainder = I::Remainder(UInt8);
    AssociatedFunction u8__addition = I::Addition(UInt8);
    AssociatedFunction u8__subtraction = I::Subtraction(UInt8);
    AssociatedFunction u8__right_shift = I::RightShift(UInt8);
    AssociatedFunction u8__left_shift = I::LeftShift(UInt8);
    AssociatedFunction u8__less_than = I::LessThan(UInt8);
    AssociatedFunction u8__greater_than = I::GreaterThan(UInt8);
    AssociatedFunction u8__less_than_equals = I::LessThanEquals(UInt8);
    AssociatedFunction u8__greater_than_equals = I::GreaterThanEquals(UInt8);
    AssociatedFunction u8__equals = I::Equals(UInt8);
    AssociatedFunction u8__not_equals = I::NotEquals(UInt8);
    AssociatedFunction u8__AND = I::AND(UInt8);
    AssociatedFunction u8__XOR = I::XOR(UInt8);
    AssociatedFunction u8__OR = I::OR(UInt8);

    AssociatedFunction u16__from_i8 = I::FromInt8(UInt16);
    AssociatedFunction u16__from_i16 = I::FromInt16(UInt16);
    AssociatedFunction u16__from_i32 = I::FromInt32(UInt16);
    AssociatedFunction u16__from_i64 = I::FromInt64(UInt16);
    AssociatedFunction u16__from_u8 = I::FromUInt8(UInt16);
    AssociatedFunction u16__from_u32 = I::FromUInt32(UInt16);
    AssociatedFunction u16__from_u64 = I::FromUInt64(UInt16);
    AssociatedFunction u16__from_int = I::FromInt(UInt16);
    AssociatedFunction u16__from_f32 = I::FromFloat32(UInt16);
    AssociatedFunction u16__from_f64 = I::FromFloat64(UInt16);
    AssociatedFunction u16__unary_plus = I::UnaryPlus(UInt16);
    AssociatedFunction u16__unary_negation = I::UnaryNegation(UInt16);
    AssociatedFunction u16__one_complement = I::OneComplement(UInt16);
    AssociatedFunction u16__multiplication = I::Multiplication(UInt16);
    AssociatedFunction u16__division = I::Division(UInt16);
    AssociatedFunction u16__remainder = I::Remainder(UInt16);
    AssociatedFunction u16__addition = I::Addition(UInt16);
    AssociatedFunction u16__subtraction = I::Subtraction(UInt16);
    AssociatedFunction u16__right_shift = I::RightShift(UInt16);
    AssociatedFunction u16__left_shift = I::LeftShift(UInt16);
    AssociatedFunction u16__less_than = I::LessThan(UInt16);
    AssociatedFunction u16__greater_than = I::GreaterThan(UInt16);
    AssociatedFunction u16__less_than_equals = I::LessThanEquals(UInt16);
    AssociatedFunction u16__greater_than_equals = I::GreaterThanEquals(UInt16);
    AssociatedFunction u16__equals = I::Equals(UInt16);
    AssociatedFunction u16__not_equals = I::NotEquals(UInt16);
    AssociatedFunction u16__AND = I::AND(UInt16);
    AssociatedFunction u16__XOR = I::XOR(UInt16);
    AssociatedFunction u16__OR = I::OR(UInt16);

    AssociatedFunction u32__from_i8 = I::FromInt8(UInt32);
    AssociatedFunction u32__from_i16 = I::FromInt16(UInt32);
    AssociatedFunction u32__from_i32 = I::FromInt32(UInt32);
    AssociatedFunction u32__from_i64 = I::FromInt64(UInt32);
    AssociatedFunction u32__from_u8 = I::FromUInt8(UInt32);
    AssociatedFunction u32__from_u16 = I::FromUInt16(UInt32);
    AssociatedFunction u32__from_u64 = I::FromUInt64(UInt32);
    AssociatedFunction u32__from_int = I::FromInt(UInt32);
    AssociatedFunction u32__from_f32 = I::FromFloat32(UInt32);
    AssociatedFunction u32__from_f64 = I::FromFloat64(UInt32);
    AssociatedFunction u32__unary_plus = I::UnaryPlus(UInt32);
    AssociatedFunction u32__unary_negation = I::UnaryNegation(UInt32);
    AssociatedFunction u32__one_complement = I::OneComplement(UInt32);
    AssociatedFunction u32__multiplication = I::Multiplication(UInt32);
    AssociatedFunction u32__division = I::Division(UInt32);
    AssociatedFunction u32__remainder = I::Remainder(UInt32);
    AssociatedFunction u32__addition = I::Addition(UInt32);
    AssociatedFunction u32__subtraction = I::Subtraction(UInt32);
    AssociatedFunction u32__right_shift = I::RightShift(UInt32);
    AssociatedFunction u32__left_shift = I::LeftShift(UInt32);
    AssociatedFunction u32__less_than = I::LessThan(UInt32);
    AssociatedFunction u32__greater_than = I::GreaterThan(UInt32);
    AssociatedFunction u32__less_than_equals = I::LessThanEquals(UInt32);
    AssociatedFunction u32__greater_than_equals = I::GreaterThanEquals(UInt32);
    AssociatedFunction u32__equals = I::Equals(UInt32);
    AssociatedFunction u32__not_equals = I::NotEquals(UInt32);
    AssociatedFunction u32__AND = I::AND(UInt32);
    AssociatedFunction u32__XOR = I::XOR(UInt32);
    AssociatedFunction u32__OR = I::OR(UInt32);

    AssociatedFunction u64__from_i8 = I::FromInt8(UInt64);
    AssociatedFunction u64__from_i16 = I::FromInt16(UInt64);
    AssociatedFunction u64__from_i32 = I::FromInt32(UInt64);
    AssociatedFunction u64__from_i64 = I::FromInt64(UInt64);
    AssociatedFunction u64__from_u8 = I::FromUInt8(UInt64);
    AssociatedFunction u64__from_u16 = I::FromUInt16(UInt64);
    AssociatedFunction u64__from_u32 = I::FromUInt32(UInt64);
    AssociatedFunction u64__from_int = I::FromInt(UInt64);
    AssociatedFunction u64__from_f32 = I::FromFloat32(UInt64);
    AssociatedFunction u64__from_f64 = I::FromFloat64(UInt64);
    AssociatedFunction u64__unary_plus = I::UnaryPlus(UInt64);
    AssociatedFunction u64__unary_negation = I::UnaryNegation(UInt64);
    AssociatedFunction u64__one_complement = I::OneComplement(UInt64);
    AssociatedFunction u64__multiplication = I::Multiplication(UInt64);
    AssociatedFunction u64__division = I::Division(UInt64);
    AssociatedFunction u64__remainder = I::Remainder(UInt64);
    AssociatedFunction u64__addition = I::Addition(UInt64);
    AssociatedFunction u64__subtraction = I::Subtraction(UInt64);
    AssociatedFunction u64__right_shift = I::RightShift(UInt64);
    AssociatedFunction u64__left_shift = I::LeftShift(UInt64);
    AssociatedFunction u64__less_than = I::LessThan(UInt64);
    AssociatedFunction u64__greater_than = I::GreaterThan(UInt64);
    AssociatedFunction u64__less_than_equals = I::LessThanEquals(UInt64);
    AssociatedFunction u64__greater_than_equals = I::GreaterThanEquals(UInt64);
    AssociatedFunction u64__equals = I::Equals(UInt64);
    AssociatedFunction u64__not_equals = I::NotEquals(UInt64);
    AssociatedFunction u64__AND = I::AND(UInt64);
    AssociatedFunction u64__XOR = I::XOR(UInt64);
    AssociatedFunction u64__OR = I::OR(UInt64);

    AssociatedFunction int__from_i8 = I::FromInt8(Int);
    AssociatedFunction int__from_i16 = I::FromInt16(Int);
    AssociatedFunction int__from_i32 = I::FromInt32(Int);
    AssociatedFunction int__from_i64 = I::FromInt64(Int);
    AssociatedFunction int__from_u8 = I::FromUInt8(Int);
    AssociatedFunction int__from_u16 = I::FromUInt16(Int);
    AssociatedFunction int__from_u32 = I::FromUInt32(Int);
    AssociatedFunction int__from_u64 = I::FromUInt64(Int);
    AssociatedFunction int__from_f32 = I::FromFloat32(Int);
    AssociatedFunction int__from_f64 = I::FromFloat64(Int);
    AssociatedFunction int__unary_plus = I::UnaryPlus(Int);
    AssociatedFunction int__unary_negation = I::UnaryNegation(Int);
    AssociatedFunction int__one_complement = I::OneComplement(Int);
    AssociatedFunction int__multiplication = I::Multiplication(Int);
    AssociatedFunction int__division = I::Division(Int);
    AssociatedFunction int__remainder = I::Remainder(Int);
    AssociatedFunction int__addition = I::Addition(Int);
    AssociatedFunction int__subtraction = I::Subtraction(Int);
    AssociatedFunction int__right_shift = I::RightShift(Int);
    AssociatedFunction int__left_shift = I::LeftShift(Int);
    AssociatedFunction int__less_than = I::LessThan(Int);
    AssociatedFunction int__greater_than = I::GreaterThan(Int);
    AssociatedFunction int__less_than_equals = I::LessThanEquals(Int);
    AssociatedFunction int__greater_than_equals = I::GreaterThanEquals(Int);
    AssociatedFunction int__equals = I::Equals(Int);
    AssociatedFunction int__not_equals = I::NotEquals(Int);
    AssociatedFunction int__AND = I::AND(Int);
    AssociatedFunction int__XOR = I::XOR(Int);
    AssociatedFunction int__OR = I::OR(Int);

    AssociatedFunction f32__from_i8 = F::FromInt8(Float32);
    AssociatedFunction f32__from_i16 = F::FromInt16(Float32);
    AssociatedFunction f32__from_i32 = F::FromInt32(Float32);
    AssociatedFunction f32__from_i64 = F::FromInt64(Float32);
    AssociatedFunction f32__from_u8 = F::FromUInt8(Float32);
    AssociatedFunction f32__from_u16 = F::FromUInt16(Float32);
    AssociatedFunction f32__from_u32 = F::FromUInt32(Float32);
    AssociatedFunction f32__from_u64 = F::FromUInt64(Float32);
    AssociatedFunction f32__from_int = F::FromInt(Float32);
    AssociatedFunction f32__from_f64 =
    {
        Float32,
        "from_f64",
        [](Emitter& t_emitter)
        {
            auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFPTrunc(
                t_emitter.EmitLoadArgument(0, Float64.GetIRType(t_emitter)),
                llvm::Type::getFloatTy(t_emitter.GetContext())
            );

            t_emitter.GetBlockBuilder().Builder.CreateRet(value);
        }
    };
    AssociatedFunction f32__unary_plus = F::UnaryPlus(Float32);
    AssociatedFunction f32__unary_negation = F::UnaryNegation(Float32);
    AssociatedFunction f32__multiplication = F::Multiplication(Float32);
    AssociatedFunction f32__division = F::Division(Float32);
    AssociatedFunction f32__remainder = F::Remainder(Float32);
    AssociatedFunction f32__addition = F::Addition(Float32);
    AssociatedFunction f32__subtraction = F::Subtraction(Float32);
    AssociatedFunction f32__less_than = F::LessThan(Float32);
    AssociatedFunction f32__greater_than = F::GreaterThan(Float32);
    AssociatedFunction f32__less_than_equals = F::LessThanEquals(Float32);
    AssociatedFunction f32__greater_than_equals = F::GreaterThanEquals(Float32);
    AssociatedFunction f32__equals = F::Equals(Float32);
    AssociatedFunction f32__not_equals = F::NotEquals(Float32);

    AssociatedFunction f64__from_i8 = F::FromInt8(Float64);
    AssociatedFunction f64__from_i16 = F::FromInt16(Float64);
    AssociatedFunction f64__from_i32 = F::FromInt32(Float64);
    AssociatedFunction f64__from_i64 = F::FromInt64(Float64);
    AssociatedFunction f64__from_u8 = F::FromUInt8(Float64);
    AssociatedFunction f64__from_u16 = F::FromUInt16(Float64);
    AssociatedFunction f64__from_u32 = F::FromUInt32(Float64);
    AssociatedFunction f64__from_u64 = F::FromUInt64(Float64);
    AssociatedFunction f64__from_int = F::FromInt(Float64);
    AssociatedFunction f64__from_f32 =
    {
        Float64,
        "from_f32",
        [](Emitter& t_emitter)
        {
            auto* const value = t_emitter.GetBlockBuilder().Builder.CreateFPExt(
                t_emitter.EmitLoadArgument(0, Float32.GetIRType(t_emitter)),
                llvm::Type::getDoubleTy(t_emitter.GetContext())
            );

            t_emitter.GetBlockBuilder().Builder.CreateRet(value);
        }
    };
    AssociatedFunction f64__unary_plus = F::UnaryPlus(Float64);
    AssociatedFunction f64__unary_negation = F::UnaryNegation(Float64);
    AssociatedFunction f64__multiplication = F::Multiplication(Float64);
    AssociatedFunction f64__division = F::Division(Float64);
    AssociatedFunction f64__remainder = F::Remainder(Float64);
    AssociatedFunction f64__addition = F::Addition(Float64);
    AssociatedFunction f64__subtraction = F::Subtraction(Float64);
    AssociatedFunction f64__less_than = F::LessThan(Float64);
    AssociatedFunction f64__greater_than = F::GreaterThan(Float64);
    AssociatedFunction f64__less_than_equals = F::LessThanEquals(Float64);
    AssociatedFunction f64__greater_than_equals = F::GreaterThanEquals(Float64);
    AssociatedFunction f64__equals = F::Equals(Float64);
    AssociatedFunction f64__not_equals = F::NotEquals(Float64);

    AssociatedFunctionTemplate StrongPointer__new =
    {
        StrongPointer,
        "new"
    };

    AssociatedFunctionTemplate StrongPointer__value =
    {
        StrongPointer,
        "value"
    };

    static const std::vector<Type*> AllTypes
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

    static const std::vector<TypeTemplate*> AllTypeTemplates
    {
        &Reference,
        &StrongPointer,
    };

    static const std::vector<Function*> AllFunctions
    {
        &print_int,
        &print_ptr,

        &alloc,
        &dealloc,
        &copy,
    };

    static const std::vector<Function*> AllFunctionTemplates
    {
    };

    static const std::vector<AssociatedFunction*> AllAssociatedFunctions
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

    static const std::vector<AssociatedFunctionTemplate*> AllAssociatedFunctionTemplates
    {
        &StrongPointer__new,
        &StrongPointer__value,
    };

    static auto GetAll() -> std::vector<IBase*>
    {
        std::vector<IBase*> all{};

        all.reserve(
            AllTypes.size() +
            AllTypeTemplates.size() +
            AllFunctions.size() + 
            AllFunctionTemplates.size() +
            AllAssociatedFunctions.size() + 
            AllAssociatedFunctionTemplates.size()
        );

        all.insert(end(all), begin(AllTypes), end(AllTypes));
        all.insert(end(all), begin(AllTypeTemplates), end(AllTypeTemplates));
        all.insert(end(all), begin(AllFunctions), end(AllFunctions));
        all.insert(end(all), begin(AllFunctionTemplates), end(AllFunctionTemplates));
        all.insert(end(all), begin(AllAssociatedFunctions), end(AllAssociatedFunctions));
        all.insert(end(all), begin(AllAssociatedFunctionTemplates), end(AllAssociatedFunctionTemplates));

        return all;
    }

    static auto CreateImplicitConversionMap() -> std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>
    {
        return
        {
            {
                Int8.GetSymbol(),
                { 
                }
            },
            {
                Int16.GetSymbol(),
                {
                    { Int8.GetSymbol(), i16__from_i8.GetSymbol() },
                    { UInt8.GetSymbol(), i16__from_u8.GetSymbol() },
                }
            },
            {
                Int32.GetSymbol(),
                {
                    { Int8.GetSymbol(), i32__from_i8.GetSymbol() },
                    { Int16.GetSymbol(), i32__from_i16.GetSymbol() },
                    { UInt8.GetSymbol(), i32__from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), i32__from_u16.GetSymbol() },
                }
            },
            {
                Int64.GetSymbol(),
                {
                    { Int8.GetSymbol(), i64__from_i8.GetSymbol() },
                    { Int16.GetSymbol(), i64__from_i16.GetSymbol() },
                    { Int32.GetSymbol(), i64__from_i32.GetSymbol() },
                    { UInt8.GetSymbol(), i64__from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), i64__from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), i64__from_u32.GetSymbol() },
                }
            },
            {
                UInt8.GetSymbol(),
                {
                }
            },
            {
                UInt16.GetSymbol(),
                {
                    { UInt8.GetSymbol(), u16__from_u8.GetSymbol() },
                }
            },
            {
                UInt32.GetSymbol(),
                {
                    { UInt8.GetSymbol(), u32__from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), u32__from_u16.GetSymbol() },
                }
            },
            {
                UInt64.GetSymbol(),
                {
                    { UInt8.GetSymbol(), u64__from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), u64__from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), u64__from_u32.GetSymbol() },
                }
            },
            {
                Float32.GetSymbol(),
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
                }
            },
            {
                Float64.GetSymbol(),
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
                }
            },
        };
    }

    // TODO: Finish map.
    static auto CreateExplicitConversionMap() -> std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>
    {
        return
        {
            {
                Int8.GetSymbol(),
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
                }
            },
            {
                Int16.GetSymbol(),
                {
                    { Int32.GetSymbol(), i16__from_i32.GetSymbol() },
                    { Int64.GetSymbol(), i16__from_i64.GetSymbol() },
                    { UInt16.GetSymbol(), i16__from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), i16__from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), i16__from_u64.GetSymbol() },
                    { Int.GetSymbol(), i16__from_int.GetSymbol() },
                    { Float32.GetSymbol(), i16__from_f32.GetSymbol() },
                    { Float64.GetSymbol(), i16__from_f64.GetSymbol() },
                }
            },
            {
                Int32.GetSymbol(),
                {
                    { Int64.GetSymbol(), i32__from_i64.GetSymbol() },
                    { UInt32.GetSymbol(), i32__from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), i32__from_u64.GetSymbol() },
                    { Int.GetSymbol(), i32__from_int.GetSymbol() },
                    { Float32.GetSymbol(), i32__from_f32.GetSymbol() },
                    { Float64.GetSymbol(), i32__from_f64.GetSymbol() },
                }
            },
            {
                Int64.GetSymbol(),
                {
                    { UInt64.GetSymbol(), i64__from_u64.GetSymbol() },
                    { Int.GetSymbol(), i64__from_int.GetSymbol() },
                    { Float32.GetSymbol(), i64__from_f32.GetSymbol() },
                    { Float64.GetSymbol(), i64__from_f64.GetSymbol() },
                }
            },
            {
                UInt8.GetSymbol(),
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
                }
            },
            {
                UInt16.GetSymbol(),
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
                }
            },
            {
                UInt32.GetSymbol(),
                {
                    { Int8.GetSymbol(), u32__from_i8.GetSymbol() },
                    { Int16.GetSymbol(), u32__from_i16.GetSymbol() },
                    { Int32.GetSymbol(), u32__from_i32.GetSymbol() },
                    { Int64.GetSymbol(), u32__from_i64.GetSymbol() },
                    { UInt64.GetSymbol(), u32__from_u64.GetSymbol() },
                    { Int.GetSymbol(), u32__from_int.GetSymbol() },
                    { Float32.GetSymbol(), u32__from_f32.GetSymbol() },
                    { Float64.GetSymbol(), u32__from_f64.GetSymbol() },
                }
            },
            {
                UInt64.GetSymbol(),
                {
                    { Int8.GetSymbol(), u64__from_i8.GetSymbol() },
                    { Int16.GetSymbol(), u64__from_i16.GetSymbol() },
                    { Int32.GetSymbol(), u64__from_i32.GetSymbol() },
                    { Int64.GetSymbol(), u64__from_i64.GetSymbol() },
                    { Int.GetSymbol(), u64__from_int.GetSymbol() },
                    { Float32.GetSymbol(), u64__from_f32.GetSymbol() },
                    { Float64.GetSymbol(), u64__from_f64.GetSymbol() },
                }
            },
            {
                Int.GetSymbol(),
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
                }
            },
            {
                Float32.GetSymbol(),
                {
                    { Float64.GetSymbol(), f32__from_f64.GetSymbol() },
                }
            },
            {
                Float64.GetSymbol(),
                {
                }
            },
        };
    }

    static std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>> ImplicitFromOperatorMap{};
    static std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>> ExplicitFromOperatorMap{};

    auto InitializeSymbols() -> Expected<void>
    {
        const auto all = GetAll();
        
        ACE_TRY_VOID(TransformExpectedVector(all, [&]
        (IBase* const t_symbol)
        {
            return t_symbol->Initialize();
        }));

        ImplicitFromOperatorMap = CreateImplicitConversionMap();
        ExplicitFromOperatorMap = CreateExplicitConversionMap();

        return ExpectedVoid;
    }

    auto GetIRTypeSymbolMap(Emitter& t_emitter) -> std::unordered_map<Symbol::Type::IBase*, llvm::Type*>
    {
        std::unordered_map<Symbol::Type::IBase*, llvm::Type*> map{};
        std::for_each(begin(AllTypes), end(AllTypes), [&]
        (const Type* const t_symbol)
        {
            if (!t_symbol->HasIRType())
                return;

            map[t_symbol->GetSymbol()] = t_symbol->GetIRType(t_emitter);
        });

        return map;
    }

    auto GetImplicitFromOperatorMap() -> std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>
    {
        return ImplicitFromOperatorMap;
    }

    auto GetExplicitFromOperatorMap() -> std::unordered_map<Symbol::Type::IBase*, std::unordered_map<Symbol::Type::IBase*, Symbol::Function*>>
    {
        return ExplicitFromOperatorMap;
    }
}
