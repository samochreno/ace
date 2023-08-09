#include "Natives.hpp"

#include <vector>
#include <optional>
#include <set>
#include <algorithm>

#include "Diagnostic.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "Emittable.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Emitter.hpp"
#include "SpecialIdent.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class FunctionEmittableBody : public IEmittable<void>
    {
    public:
        FunctionEmittableBody(const FunctionBodyEmitter& bodyEmitter)
            : m_BodyEmitter{ bodyEmitter }
        {
        }
        virtual ~FunctionEmittableBody() = default;

        auto Emit(Emitter& emitter) const -> void final
        {
            m_BodyEmitter(emitter);
        }

   private:
        FunctionBodyEmitter m_BodyEmitter;
    };

    static auto CreateSymbolName(
        const SrcLocation& srcLocation,
        const std::vector<const char*>& nameSectionStrings
    ) -> SymbolName
    {
        std::vector<SymbolNameSection> sections{};
        std::transform(
            begin(nameSectionStrings),
            end  (nameSectionStrings),
            back_inserter(sections),
            [&](const char* const nameSectionString)
            {
                return SymbolNameSection
                {
                    Ident{ srcLocation, nameSectionString }
                };
            }
        );

        return SymbolName{ sections, SymbolNameResolutionScope::Global };
    }

    static auto CreateAssociatedSymbolName(
        const SrcLocation& srcLocation,
        const INative& type,
        const char* const name
    ) -> SymbolName
    {
        auto symbolName = type.CreateFullyQualifiedName(srcLocation);
        symbolName.Sections.emplace_back(Ident{ srcLocation, name });
        return symbolName;
    }

    NativeType::NativeType(
        Compilation* const compilation,
        std::vector<const char*>&& nameSectionStrings,
        std::optional<std::function<llvm::Type*(llvm::LLVMContext&)>>&& irTypeGetter,
        const TypeSizeKind sizeKind,
        const NativeCopyabilityKind copyabilityKind
    ) : m_Compilation{ compilation },
        m_NameSectionStrings{ std::move(nameSectionStrings) },
        m_IRTypeGetter{ std::move(irTypeGetter) },
        m_IsSized{ sizeKind == TypeSizeKind::Sized },
        m_IsTriviallyCopyable{ copyabilityKind == NativeCopyabilityKind::Trivial },
        m_Symbol
        {
            [this]()
            {
                const auto globalScope = GetCompilation()->GetGlobalScope();
                auto* const symbol = globalScope->ResolveStaticSymbol<ITypeSymbol>(
                    CreateFullyQualifiedName(SrcLocation{})
                ).Unwrap();

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

                return symbol;
            }
        }
    {
        ACE_ASSERT(
            (sizeKind == TypeSizeKind::Sized) ||
            (sizeKind == TypeSizeKind::Unsized)
        );

        ACE_ASSERT(
            (copyabilityKind == NativeCopyabilityKind::Trivial) ||
            (copyabilityKind == NativeCopyabilityKind::NonTrivial)
        );
    }

    auto NativeType::GetCompilation() const -> Compilation*
    {
        return m_Compilation;
    }

    auto NativeType::CreateFullyQualifiedName(
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        return CreateSymbolName(
            srcLocation,
            m_NameSectionStrings
        );
    }

    auto NativeType::GetSymbol() const -> ITypeSymbol*
    {
        return m_Symbol.Get();
    }

    auto NativeType::GetGenericSymbol() const -> ISymbol*
    {
        return GetSymbol();
    }

    auto NativeType::HasIRType() const -> bool
    {
        return m_IRTypeGetter.has_value();
    }

    auto NativeType::GetIRType(llvm::LLVMContext& context) const -> llvm::Type*
    {
        ACE_ASSERT(HasIRType());
        return m_IRTypeGetter.value()(context);
    }

    NativeTypeTemplate::NativeTypeTemplate(
        Compilation* const compilation,
        std::vector<const char*>&& nameSectionStrings
    ) : m_Compilation{ compilation },
        m_NameSectionStrings{ std::move(nameSectionStrings) },
        m_Symbol
        {
            [this]()
            {
                auto name = CreateFullyQualifiedName(SrcLocation{});
                name.Sections.back().Name.String = SpecialIdent::CreateTemplate(
                    name.Sections.back().Name.String
                );

                const auto globalScope = GetCompilation()->GetGlobalScope();
                return globalScope->ResolveStaticSymbol<TypeTemplateSymbol>(
                    name
                ).Unwrap();
            }
        }
    {
    }

    auto NativeTypeTemplate::GetCompilation() const -> Compilation*
    {
        return m_Compilation;
    }

    auto NativeTypeTemplate::CreateFullyQualifiedName(
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        return CreateSymbolName(
            srcLocation,
            m_NameSectionStrings
        );
    }

    auto NativeTypeTemplate::GetSymbol() const -> TypeTemplateSymbol*
    {
        return m_Symbol.Get();
    }

    auto NativeTypeTemplate::GetGenericSymbol() const -> ISymbol*
    {
        return GetSymbol();
    }

    NativeFunction::NativeFunction(
        Compilation* const compilation,
        std::vector<const char*>&& nameSectionStrings,
        FunctionBodyEmitter&& bodyEmitter
    ) : m_Compilation{ compilation },
        m_NameSectionStrings{ std::move(nameSectionStrings) },
        m_BodyEmitter{ std::move(bodyEmitter) },
        m_Symbol
        {
            [this]()
            {
                const auto globalScope = GetCompilation()->GetGlobalScope();
                auto* const symbol = globalScope->ResolveStaticSymbol<FunctionSymbol>(
                    CreateFullyQualifiedName(SrcLocation{})
                ).Unwrap();

                symbol->BindBody(std::make_shared<FunctionEmittableBody>(
                    m_BodyEmitter
                ));

                return symbol;
            }
        }
    {
    }

    auto NativeFunction::GetCompilation() const -> Compilation*
    {
        return m_Compilation;
    }

    auto NativeFunction::CreateFullyQualifiedName(
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        return CreateSymbolName(
            srcLocation,
            m_NameSectionStrings
        );
    }

    auto NativeFunction::GetSymbol() const -> FunctionSymbol*
    {
        return m_Symbol.Get();
    }

    auto NativeFunction::GetGenericSymbol() const -> ISymbol*
    {
        return GetSymbol();
    }

    NativeFunctionTemplate::NativeFunctionTemplate(
        Compilation* const compilation,
        std::vector<const char*>&& nameSectionStrings
    ) : m_Compilation{ compilation },
        m_NameSectionStrings{ std::move(nameSectionStrings) },
        m_Symbol
        {
            [this]()
            {
                const auto globalScope = GetCompilation()->GetGlobalScope();
                return globalScope->ResolveStaticSymbol<FunctionTemplateSymbol>(
                    CreateFullyQualifiedName(SrcLocation{})
                ).Unwrap();
            }
        }
    {
    }

    auto NativeFunctionTemplate::GetCompilation() const -> Compilation*
    {
        return m_Compilation;
    }

    auto NativeFunctionTemplate::CreateFullyQualifiedName(
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        return CreateSymbolName(
            srcLocation,
            m_NameSectionStrings
        );
    }

    auto NativeFunctionTemplate::GetSymbol() const -> FunctionTemplateSymbol*
    {
        return m_Symbol.Get();
    }

    auto NativeFunctionTemplate::GetGenericSymbol() const -> ISymbol*
    {
        return GetSymbol();
    }

    NativeAssociatedFunction::NativeAssociatedFunction(
        const INative& type,
        const char* const name,
        FunctionBodyEmitter&& bodyEmitter
    ) : m_Type{ type },
        m_Name{ name },
        m_BodyEmitter{ std::move(bodyEmitter) },
        m_Symbol
        {
            [this]()
            {
                const auto globalScope = GetCompilation()->GetGlobalScope();
                auto* const symbol = globalScope->ResolveStaticSymbol<FunctionSymbol>(
                    CreateFullyQualifiedName(SrcLocation{})
                ).Unwrap();

                symbol->BindBody(std::make_shared<FunctionEmittableBody>(
                    m_BodyEmitter
                ));

                return symbol;
            }
        }
    {
    }

    auto NativeAssociatedFunction::GetCompilation() const -> Compilation*
    {
        return m_Type.GetCompilation();
    }

    auto NativeAssociatedFunction::CreateFullyQualifiedName(
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        return CreateAssociatedSymbolName(
            srcLocation,
            m_Type,
            m_Name
        );
    }

    auto NativeAssociatedFunction::GetSymbol() const -> FunctionSymbol*
    {
        return m_Symbol.Get();
    }

    auto NativeAssociatedFunction::GetGenericSymbol() const -> ISymbol*
    {
        return GetSymbol();
    }

    NativeAssociatedFunctionTemplate::NativeAssociatedFunctionTemplate(
        const INative& type,
        const char* const name
    ) : m_Type{ type },
        m_Name{ name },
        m_Symbol
        {
            [this]()
            {
                const auto globalScope = GetCompilation()->GetGlobalScope();
                return globalScope->ResolveStaticSymbol<FunctionTemplateSymbol>(
                    CreateFullyQualifiedName(SrcLocation{})
                ).Unwrap();
            }
        }
    {
    }

    auto NativeAssociatedFunctionTemplate::GetCompilation() const -> Compilation*
    {
        return m_Type.GetCompilation();
    }

    auto NativeAssociatedFunctionTemplate::CreateFullyQualifiedName(
        const SrcLocation& srcLocation
    ) const -> SymbolName
    {
        return CreateAssociatedSymbolName(
            srcLocation,
            m_Type,
            m_Name
        );
    }

    auto NativeAssociatedFunctionTemplate::GetSymbol() const -> FunctionTemplateSymbol*
    {
        return m_Symbol.Get();
    }

    auto NativeAssociatedFunctionTemplate::GetGenericSymbol() const -> ISymbol*
    {
        return GetSymbol();
    }

    namespace I
    {        
        static auto FromIntImpl(
            const char* const name,
            const NativeType& fromType,
            const NativeType& targetType
        ) -> NativeAssociatedFunction
        {
            return
            {
                targetType,
                name,
                [&](Emitter& emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        auto* const fromIRType = fromType.GetIRType(
                            emitter.GetContext()
                        );
                        auto* const toIRType = targetType.GetIRType(
                            emitter.GetContext()
                        );

                        if (fromType.GetCompilation()->GetNatives().IsIntTypeSigned(fromType))
                        {
                            return emitter.GetBlockBuilder().Builder.CreateSExtOrTrunc(
                                emitter.EmitLoadArg(0, fromIRType),
                                toIRType
                            );
                        }
                        else
                        {
                            return emitter.GetBlockBuilder().Builder.CreateZExtOrTrunc(
                                emitter.EmitLoadArg(0, fromIRType),
                                toIRType
                            );
                        }
                    }();

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto FromInt8(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i8",
                natives->Int8,
                targetType
            );
        }

        static auto FromInt16(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i16",
                natives->Int16,
                targetType
            );
        }

        static auto FromInt32(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i32",
                natives->Int32,
                targetType
            );
        }

        static auto FromInt64(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i64", 
                natives->Int64,
                targetType
            );
        }

        static auto FromUInt8(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u8",
                natives->UInt8,
                targetType
            );
        }

        static auto FromUInt16(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u16",
                natives->UInt16,
                targetType
            );
        }

        static auto FromUInt32(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u32",
                natives->UInt32,
                targetType
            );
        }

        static auto FromUInt64(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u64",
                natives->UInt64,
                targetType
            );
        }

        static auto FromInt(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_int",
                natives->Int,
                targetType
            );
        }

        static auto FromFloat(
            const char* const name,
            const NativeType& fromType,
            const NativeType& targetType
        ) -> NativeAssociatedFunction
        {
            return
            {
                targetType,
                name,
                [&](Emitter& emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        auto* const fromIRType = fromType.GetIRType(
                            emitter.GetContext()
                        );
                        auto* const toIRType = targetType.GetIRType(
                            emitter.GetContext()
                        );

                        if (fromType.GetCompilation()->GetNatives().IsIntTypeSigned(targetType))
                        {
                            return emitter.GetBlockBuilder().Builder.CreateFPToSI(
                                emitter.EmitLoadArg(0, fromIRType),
                                toIRType
                            );
                        }
                        else
                        {
                            return emitter.GetBlockBuilder().Builder.CreateFPToUI(
                                emitter.EmitLoadArg(0, fromIRType),
                                toIRType
                            );
                        }
                    }();

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto FromFloat32(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromFloat(
                "from_f32",
                natives->Float32,
                targetType
            );
        }

        static auto FromFloat64(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromFloat(
                "from_f64",
                natives->Float64,
                targetType
            );
        }

        static auto Division(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Division,
                [&](Emitter& emitter)
                {
                    auto* const value = [&]() -> llvm::Value*
                    {
                        auto* const selfIRType = selfType.GetIRType(
                            emitter.GetContext()
                        );

                        if (selfType.GetCompilation()->GetNatives().IsIntTypeSigned(selfType))
                        {
                            return emitter.GetBlockBuilder().Builder.CreateSDiv(
                                emitter.EmitLoadArg(0, selfIRType),
                                emitter.EmitLoadArg(1, selfIRType)
                            );
                        }
                        else
                        {
                            return emitter.GetBlockBuilder().Builder.CreateUDiv(
                                emitter.EmitLoadArg(0, selfIRType),
                                emitter.EmitLoadArg(1, selfIRType)
                            );
                        }
                    }();

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Remainder(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Remainder,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (selfType.GetCompilation()->GetNatives().IsIntTypeSigned(selfType))
                        {
                            return emitter.GetBlockBuilder().Builder.CreateSRem(
                                emitter.EmitLoadArg(0, selfIRType),
                                emitter.EmitLoadArg(1, selfIRType)
                            );
                        }
                        else
                        {
                            return emitter.GetBlockBuilder().Builder.CreateURem(
                                emitter.EmitLoadArg(0, selfIRType),
                                emitter.EmitLoadArg(1, selfIRType)
                            );
                        }
                    }();

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto UnaryPlus(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::UnaryPlus,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(
                        emitter.EmitLoadArg(0, selfIRType)
                    );
                }
            };
        }

        static auto UnaryNegation(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::UnaryNegation,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateMul(
                        emitter.EmitLoadArg(0, selfIRType),
                        llvm::ConstantInt::get(selfIRType, -1)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };

        }

        static auto OneComplement(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::OneComplement,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateXor(
                        emitter.EmitLoadArg(0, selfIRType),
                        llvm::ConstantInt::get(selfIRType, -1)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Multiplication(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return 
            {
                selfType,
                SpecialIdent::Op::Multiplication,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateMul(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Addition(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Addition,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateAdd(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Subtraction(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Subtraction,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateSub(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto RightShift(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::RightShift,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateAShr(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        };

        static auto LeftShift(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::LeftShift,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateShl(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThan(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::LessThan,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLT,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThan(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::GreaterThan,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGT,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThanEquals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::LessThanEquals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SLE,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThanEquals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::GreaterThanEquals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_SGE,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Equals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Equals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_EQ,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto NotEquals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::NotEquals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_NE,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto AND(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::AND,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateAnd(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto XOR(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::XOR,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateXor(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto OR(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::OR,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateOr(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }
    }

    namespace F
    {
        static auto FromIntImpl(
            const char* const name,
            const NativeType& fromType,
            const NativeType& targetType
        ) -> NativeAssociatedFunction
        {
            return
            {
                targetType,
                name,
                [&](Emitter& emitter)
                {
                    auto* const fromIRType = fromType.GetIRType(
                        emitter.GetContext()
                    );
                    auto* const toIRType = targetType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = [&]() -> llvm::Value*
                    {
                        if (fromType.GetCompilation()->GetNatives().IsIntTypeSigned(fromType))
                        {
                            return emitter.GetBlockBuilder().Builder.CreateSIToFP(
                                emitter.EmitLoadArg(0, fromIRType),
                                toIRType
                            );
                        }
                        else
                        {
                            return emitter.GetBlockBuilder().Builder.CreateUIToFP(
                                emitter.EmitLoadArg(0, fromIRType),
                                toIRType
                            );
                        }
                    }();

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto FromInt8(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i8",
                natives->Int8,
                targetType
            );
        }

        static auto FromInt16(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i16",
                natives->Int16,
                targetType
            );
        }

        static auto FromInt32(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i32",
                natives->Int32,
                targetType
            );
        }

        static auto FromInt64(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_i64",
                natives->Int64,
                targetType
            );
        }

        static auto FromUInt8(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u8",
                natives->UInt8,
                targetType
            );
        }

        static auto FromUInt16(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u16",
                natives->UInt16,
                targetType
            );
        }

        static auto FromUInt32(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u32",
                natives->UInt32,
                targetType
            );
        }

        static auto FromUInt64(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_u64",
                natives->UInt64,
                targetType
            );
        }

        static auto FromInt(
            const NativeType& targetType,
            const Natives* const natives
        ) -> NativeAssociatedFunction
        {
            return FromIntImpl(
                "from_int",
                natives->Int,
                targetType
            );
        }

        static auto UnaryPlus(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::UnaryPlus,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(
                        emitter.EmitLoadArg(0, selfIRType)
                    );
                }
            };
        }

        static auto UnaryNegation(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::UnaryNegation,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFMul(
                        emitter.EmitLoadArg(0, selfIRType),
                        llvm::ConstantFP::get(selfIRType, -1)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Multiplication(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Multiplication,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFMul(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Division(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Division,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFDiv(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );
            
                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Remainder(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Remainder,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFRem(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Addition(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Addition,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFAdd(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Subtraction(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Subtraction,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFSub(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThan(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::LessThan,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OLT,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThan(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::GreaterThan,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OGT,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto LessThanEquals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::LessThanEquals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OLE,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto GreaterThanEquals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::GreaterThanEquals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OGE,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto Equals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::Equals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_OEQ,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }

        static auto NotEquals(
            const NativeType& selfType
        ) -> NativeAssociatedFunction
        {
            return
            {
                selfType,
                SpecialIdent::Op::NotEquals,
                [&](Emitter& emitter)
                {
                    auto* const selfIRType = selfType.GetIRType(
                        emitter.GetContext()
                    );

                    auto* const value = emitter.GetBlockBuilder().Builder.CreateFCmp(
                        llvm::CmpInst::Predicate::FCMP_ONE,
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                    emitter.GetBlockBuilder().Builder.CreateRet(value);
                }
            };
        }
    }

    Natives::Natives(
        Compilation* const compilation
    ) : Int8
        {
            compilation,
            std::vector{ "ace", "std", "Int8" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt8Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Int16
        {
            compilation,
            std::vector{ "ace", "std", "Int16" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt16Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Int32
        {
            compilation,
            std::vector{ "ace", "std", "Int32" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Int64
        {
            compilation,
            std::vector{ "ace", "std", "Int64" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt64Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        UInt8
        {
            compilation,
            std::vector{ "ace", "std", "UInt8" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt8Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        UInt16
        {
            compilation,
            std::vector{ "ace", "std", "UInt16" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt16Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        UInt32
        {
            compilation,
            std::vector{ "ace", "std", "UInt32" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        UInt64
        {
            compilation,
            std::vector{ "ace", "std", "UInt64" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt64Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Int
        {
            compilation,
            std::vector{ "ace", "std", "Int" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Float32
        {
            compilation,
            std::vector{ "ace", "std", "Float32" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getFloatTy(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Float64
        {
            compilation,
            std::vector{ "ace", "std", "Float64" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getDoubleTy(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Bool
        {
            compilation,
            std::vector{ "ace", "std", "Bool" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt1Ty(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },
        Void
        {
            compilation,
            std::vector{ "ace", "std", "Void" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getVoidTy(context);
            },
            TypeSizeKind::Unsized,
            NativeCopyabilityKind::NonTrivial,
        },
        String
        {
            compilation,
            std::vector{ "ace", "std", "Void" },
            {},
            TypeSizeKind::Sized,
            NativeCopyabilityKind::NonTrivial,
        },

        Ptr
        {
            compilation,
            std::vector{ "ace", "std", "Ptr" },
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt8PtrTy(context);
            },
            TypeSizeKind::Sized,
            NativeCopyabilityKind::Trivial,
        },

        Ref
        {
            compilation,
            std::vector{ "ace", "std", "Ref" },
        },
        StrongPtr
        {
            compilation,
            std::vector{ "ace", "std", "rc", "StrongPtr" },
        },
        WeakPtr
        {
            compilation,
            std::vector{ "ace", "std", "rc", "WeakPtr" },
        },

        print_int
        {
            compilation,
            std::vector{ "ace", "print_int" },
            [this, compilation](Emitter& emitter)
            {
                std::string string{ "%" PRId32 "\n" };

                auto* const charType = llvm::Type::getInt8Ty(
                    emitter.GetContext()
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
                    emitter.GetModule().getOrInsertGlobal("printf_string_int", stringType)
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
                    emitter.GetC().GetFunctions().GetPrintf();

                std::vector<llvm::Value*> args{};
                args.push_back(llvm::ConstantExpr::getBitCast(
                    globalDeclaration,
                    llvm::PointerType::get(charType, 0)
                ));
                args.push_back(emitter.EmitLoadArg(
                    0, 
                    Int.GetIRType(emitter.GetContext())
                ));

                emitter.GetBlockBuilder().Builder.CreateCall(
                    printfFunction,
                    args
                );

                emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        },
        print_ptr
        {
            compilation,
            std::vector{ "ace", "print_ptr" },
            [compilation](Emitter& emitter)
            {
                std::string string{ "0x%" PRIXPTR "\n" };

                auto* const charType = llvm::Type::getInt8Ty(
                    emitter.GetContext()
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
                    emitter.GetModule().getOrInsertGlobal("printf_string_ptr", stringType)
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
                    emitter.GetC().GetFunctions().GetPrintf();

                std::vector<llvm::Value*> args{};
                args.push_back(llvm::ConstantExpr::getBitCast(
                    globalDeclaration,
                    llvm::PointerType::get(charType, 0)
                ));
                args.push_back(emitter.EmitLoadArg(
                    0,
                    llvm::Type::getInt8PtrTy(emitter.GetContext())
                ));

                emitter.GetBlockBuilder().Builder.CreateCall(
                    printfFunction,
                    args
                );

                emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        },

        alloc
        {
            compilation,
            std::vector{ "ace", "std", "mem", "alloc" },
            [this](Emitter& emitter)
            {
                auto* const mallocFunction =
                    emitter.GetC().GetFunctions().GetMalloc();

                auto* const intTypeSymbol = Int.GetSymbol();
                auto* const intType = emitter.GetIRType(intTypeSymbol);

                auto* const cSizeType = mallocFunction->arg_begin()->getType();

                auto* const sizeValue = emitter.EmitLoadArg(0, intType);

                auto* const convertedSizeValue = emitter.GetBlockBuilder().Builder.CreateZExtOrTrunc(
                    sizeValue,
                    cSizeType
                );

                auto* const callInst = emitter.GetBlockBuilder().Builder.CreateCall(
                    mallocFunction,
                    { convertedSizeValue }
                );

                emitter.GetBlockBuilder().Builder.CreateRet(callInst);
            }
        },
        dealloc
        {
            compilation,
            std::vector{ "ace", "std", "mem", "dealloc" },
            [this](Emitter& emitter)
            {
                auto* const freeFunction =
                    emitter.GetC().GetFunctions().GetFree();

                auto* const ptrType = Ptr.GetIRType(emitter.GetContext());

                auto* const blockValue = emitter.EmitLoadArg(
                    0,
                    ptrType
                );

                emitter.GetBlockBuilder().Builder.CreateCall(
                    freeFunction,
                    { blockValue }
                );

                emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        },
        copy
        {
            compilation,
            std::vector{ "ace", "std", "mem", "copy" },
            [this](Emitter& emitter)
            {
                auto* const memcpyFunction =
                    emitter.GetC().GetFunctions().GetMemcpy();

                auto* const ptrType = Ptr.GetIRType(emitter.GetContext());

                auto* const intTypeSymbol = Int.GetSymbol();
                auto* const intType = emitter.GetIRType(intTypeSymbol);

                auto* const cSizeType =
                    (memcpyFunction->arg_begin() + 2)->getType();

                auto* const srcValue = emitter.EmitLoadArg(0, ptrType);
                auto* const dstValue = emitter.EmitLoadArg(1, ptrType);
                auto* const sizeValue = emitter.EmitLoadArg(2, intType);

                auto* const convertedSizeValue = emitter.GetBlockBuilder().Builder.CreateZExtOrTrunc(
                    sizeValue,
                    cSizeType
                );

                emitter.GetBlockBuilder().Builder.CreateCall(
                    memcpyFunction,
                    { dstValue, srcValue, convertedSizeValue }
                );

                emitter.GetBlockBuilder().Builder.CreateRetVoid();
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

        int_from_i8{ I::FromInt8(Int, this) },
        int_from_i16{ I::FromInt16(Int, this) },
        int_from_i32{ I::FromInt32(Int, this) },
        int_from_i64{ I::FromInt64(Int, this) },
        int_from_u8{ I::FromUInt8(Int, this) },
        int_from_u16{ I::FromUInt16(Int, this) },
        int_from_u32{ I::FromUInt32(Int, this) },
        int_from_u64{ I::FromUInt64(Int, this) },
        int_from_f32{ I::FromFloat32(Int, this) },
        int_from_f64{ I::FromFloat64(Int, this) },
        int_unary_plus{ I::UnaryPlus(Int) },
        int_unary_negation{ I::UnaryNegation(Int) },
        int_one_complement{ I::OneComplement(Int) },
        int_multiplication{ I::Multiplication(Int) },
        int_division{ I::Division(Int) },
        int_remainder{ I::Remainder(Int) },
        int_addition{ I::Addition(Int) },
        int_subtraction{ I::Subtraction(Int) },
        int_right_shift{ I::RightShift(Int) },
        int_left_shift{ I::LeftShift(Int) },
        int_less_than{ I::LessThan(Int) },
        int_greater_than{ I::GreaterThan(Int) },
        int_less_than_equals{ I::LessThanEquals(Int) },
        int_greater_than_equals{ I::GreaterThanEquals(Int) },
        int_equals{ I::Equals(Int) },
        int_not_equals{ I::NotEquals(Int) },
        int_AND{ I::AND(Int) },
        int_XOR{ I::XOR(Int) },
        int_OR{ I::OR(Int) },

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
            [this, compilation](Emitter& emitter)
            {
                auto* const float64IRType =
                    Float64.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlockBuilder().Builder.CreateFPTrunc(
                    emitter.EmitLoadArg(0, float64IRType),
                    llvm::Type::getFloatTy(emitter.GetContext())
                );

                emitter.GetBlockBuilder().Builder.CreateRet(value);
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
            [this, compilation](Emitter& emitter)
            {
                auto* const float32IRType =
                    Float32.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlockBuilder().Builder.CreateFPExt(
                    emitter.EmitLoadArg(0, float32IRType),
                    llvm::Type::getDoubleTy(emitter.GetContext())
                );

                emitter.GetBlockBuilder().Builder.CreateRet(value);
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

        StrongPtr__new
        {
            StrongPtr,
            "new"
        },
        StrongPtr__value
        {
            StrongPtr,
            "value"
        },

        WeakPtr__from
        {
            WeakPtr,
            "from"
        },

        m_Natives
        {
            [this]()
            {
                std::vector<INative*> natives{};

                const auto types = m_Types.Get();
                natives.insert(end(natives), begin(types), end(types));

                const auto typeTemplates = m_TypeTemplates.Get();
                natives.insert(
                    end(natives),
                    begin(typeTemplates),
                    end  (typeTemplates)
                );

                const auto functions = m_Functions.Get();
                natives.insert(end(natives), begin(functions), end(functions));

                const auto functionTemplates = m_FunctionTemplates.Get();
                natives.insert(
                    end(natives),
                    begin(functionTemplates),
                    end  (functionTemplates)
                );

                const auto associatedFunctions = m_AssociatedFunctions.Get();
                natives.insert(
                    end(natives),
                    begin(associatedFunctions),
                    end  (associatedFunctions)
                );

                const auto associatedFunctionTemplates = m_AssociatedFunctionTemplates.Get();
                natives.insert(
                    end(natives),
                    begin(associatedFunctionTemplates),
                    end  (associatedFunctionTemplates)
                );

                return natives;
            }
        },
        m_Types
        {
            [this]()
            {
                return std::vector
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

                    &Ptr,
                };
            }
        },
        m_TypeTemplates
        {
            [this]()
            {
                return std::vector
                {
                    &Ref,
                    &StrongPtr,
                    &WeakPtr,
                };
            }
        },
        m_Functions
        {
            [this]()
            {
                return std::vector
                {
                    &print_int,
                    &print_ptr,

                    &alloc,
                    &dealloc,
                    &copy,
                };
            }
        },
        m_FunctionTemplates
        {
            [this]()
            {
                return std::vector<NativeFunctionTemplate*>
                {
                };
            }
        },
        m_AssociatedFunctions
        {
            [this]()
            {
                return std::vector
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

                    &int_from_i8,
                    &int_from_i16,
                    &int_from_i32,
                    &int_from_i64,
                    &int_from_u8,
                    &int_from_u16,
                    &int_from_u32,
                    &int_from_u64,
                    &int_from_f32,
                    &int_from_f64,
                    &int_unary_plus,
                    &int_unary_negation,
                    &int_one_complement,
                    &int_multiplication,
                    &int_division,
                    &int_remainder,
                    &int_addition,
                    &int_subtraction,
                    &int_right_shift,
                    &int_left_shift,
                    &int_less_than,
                    &int_greater_than,
                    &int_less_than_equals,
                    &int_greater_than_equals,
                    &int_equals,
                    &int_not_equals,
                    &int_AND,
                    &int_XOR,
                    &int_OR,

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
            }
        },
        m_AssociatedFunctionTemplates
        {
            [this]()
            {
                return std::vector
                {
                    &StrongPtr__new,
                    &StrongPtr__value,

                    &WeakPtr__from,
                };
            }
        },

        m_ImplicitFromOpMap
        {
            [this]()
            {
                std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>> map{};

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

                return map;
            }
        },
        m_ExplicitFromOpMap
        {
            [this]()
            {
                std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>> map{};

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
                    { Int8.GetSymbol(), int_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), int_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), int_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), int_from_i64.GetSymbol() },
                    { UInt8.GetSymbol(), int_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), int_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), int_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), int_from_u64.GetSymbol() },
                    { Float32.GetSymbol(), int_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), int_from_f64.GetSymbol() },
                };

                map[Float32.GetSymbol()] =
                {
                    { Float64.GetSymbol(), f32__from_f64.GetSymbol() },
                };
                map[Float64.GetSymbol()] =
                {
                };

                return map;
            }
        },
        
        m_SignedIntTypesSet
        {
            [this]()
            {
                return std::set<const NativeType*>
                {
                    &Int8,
                    &Int16,
                    &Int32,
                    &Int64,
                    &Int,
                };
            }
        }
    {
    }

    auto Natives::Verify() const -> void
    {
        std::for_each(begin(m_Natives.Get()), end(m_Natives.Get()),
        [](INative* const native)
        {
            (void)native->GetGenericSymbol();
        });
    }

    auto Natives::CollectIRTypeSymbolMap(
        llvm::LLVMContext& context
    ) const -> std::unordered_map<ITypeSymbol*, llvm::Type*>
    {
        std::unordered_map<ITypeSymbol*, llvm::Type*> map{};

        std::for_each(begin(m_Types.Get()), end(m_Types.Get()),
        [&](const NativeType* const type)
        {
            if (!type->HasIRType())
            {
                return;
            }

            map[type->GetSymbol()] = type->GetIRType(context);
        });

        return map;
    }

    auto Natives::GetImplicitFromOpMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>&
    {
        return m_ImplicitFromOpMap.Get();
    }

    auto Natives::GetExplicitFromOpMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<ITypeSymbol*, FunctionSymbol*>>&
    {
        return m_ExplicitFromOpMap.Get();
    }

    auto Natives::IsIntTypeSigned(const NativeType& intType) const -> bool
    {
        return m_SignedIntTypesSet.Get().contains(&intType);
    }
}
