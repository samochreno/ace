#include "Natives.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <set>
#include <string_view>

#include "Diagnostic.hpp"
#include "Std.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Emittable.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Emitter.hpp"
#include "Op.hpp"

namespace Ace
{
    class FunctionEmittableBlock : public IEmittable<void>
    {
    public:
        FunctionEmittableBlock(
            const FunctionBlockEmitter& blockEmitter
        ) : m_BlockEmitter{ blockEmitter }
        {
        }
        virtual ~FunctionEmittableBlock() = default;

        auto Emit(Emitter& emitter) const -> void final
        {
            m_BlockEmitter(emitter);
        }
        
   private:
        FunctionBlockEmitter m_BlockEmitter;
    };

    template<typename T>
    static auto CreateSymbolName(
        const SrcLocation& srcLocation,
        const std::vector<T>& nameSectionStrings
    ) -> SymbolName
    {
        std::vector<SymbolNameSection> sections{};
        std::transform(
            begin(nameSectionStrings),
            end  (nameSectionStrings),
            back_inserter(sections),
            [&](const std::string_view nameSectionString)
            {
                return SymbolNameSection
                {
                    Ident{ srcLocation, std::string{ nameSectionString } }
                };
            }
        );

        return SymbolName{ sections, SymbolNameResolutionScope::Global };
    }

    NativeType::NativeType(
        Compilation* const compilation,
        std::vector<const char*> nameSectionStrings,
        const NativeSymbolKind kind,
        const NativeCopyabilityKind copyabilityKind,
        std::optional<std::function<llvm::Type*(llvm::LLVMContext&)>> irTypeGetter
    ) : m_Compilation{ compilation },
        m_NameSectionStrings{ std::move(nameSectionStrings) },
        m_IRTypeGetter{ std::move(irTypeGetter) },
        m_IsRoot{ kind == NativeSymbolKind::Root },
        m_IsTriviallyCopyable
        {
            copyabilityKind == NativeCopyabilityKind::Trivial
        }
    {
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
        return CreateSymbolName(srcLocation, m_NameSectionStrings);
    }

    auto NativeType::TryGetSymbol() const -> std::optional<ITypeSymbol*>
    {
        if (m_OptSymbol.has_value())
        {
            return m_OptSymbol;
        }

        const auto globalScope = GetCompilation()->GetGlobalScope();
        const auto name =
            CreateFullyQualifiedName(SrcLocation{ GetCompilation() });

        m_OptSymbol = DiagnosticBag::Create().Collect(m_IsRoot ?
            globalScope->ResolveRoot        <ITypeSymbol>(name) :
            globalScope->ResolveStaticSymbol<ITypeSymbol>(name)
        );

        if (m_OptSymbol.has_value())
        {
            if (m_IRTypeGetter.has_value())
            {
                dynamic_cast<IConcreteTypeSymbol*>(
                    m_OptSymbol.value()
                )->SetAsPrimitivelyEmittable();
            }

            if (m_IsTriviallyCopyable)
            {
                dynamic_cast<IConcreteTypeSymbol*>(
                    m_OptSymbol.value()
                )->SetAsTriviallyCopyable();
            }        
        }

        return m_OptSymbol;
    }

    auto NativeType::GetSymbol() const -> ITypeSymbol*
    {
        return TryGetSymbol().value();
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

    NativeFunction::NativeFunction(
        Compilation* const compilation,
        std::vector<std::string> nameSectionStrings,
        const NativeSymbolKind kind,
        std::optional<FunctionBlockEmitter> optBlockEmitter
    ) : m_Compilation{ compilation },
        m_NameSectionStrings{ std::move(nameSectionStrings) },
        m_IsRoot{ kind == NativeSymbolKind::Root },
        m_OptBlockEmitter{ std::move(optBlockEmitter) }
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
        return CreateSymbolName(srcLocation, m_NameSectionStrings);
    }

    auto NativeFunction::TryGetSymbol() const -> std::optional<FunctionSymbol*>
    {
        if (m_OptSymbol.has_value())
        {
            return m_OptSymbol;
        }

        const auto globalScope = GetCompilation()->GetGlobalScope();
        const auto name =
            CreateFullyQualifiedName(SrcLocation{ GetCompilation() });

        if (m_IsRoot)
        {
            m_OptSymbol = DiagnosticBag::Create().Collect(
                globalScope->ResolveRoot<FunctionSymbol>(name)
            );

            ACE_ASSERT(!m_OptBlockEmitter.has_value());
        }
        else
        {
            m_OptSymbol = DiagnosticBag::Create().Collect(
                globalScope->ResolveStaticSymbol<FunctionSymbol>(name)
            );

            if (m_OptSymbol.has_value() && m_OptBlockEmitter.has_value())
            {
                m_OptSymbol.value()->BindEmittableBlock(
                    std::make_shared<FunctionEmittableBlock>(
                        m_OptBlockEmitter.value()
                    )
                );
            }
        }

        return m_OptSymbol;
    }

    auto NativeFunction::GetSymbol() const -> FunctionSymbol*
    {
        return TryGetSymbol().value();
    }

    auto NativeFunction::GetGenericSymbol() const -> ISymbol*
    {
        return GetSymbol();
    }

    static auto CreateTypeAliasNameString(
        const Natives& natives,
        const NativeType& type
    ) -> std::string
    {
        if (&type == &natives.Int8)
        {
            return "i8";
        }
        else if (&type == &natives.Int16)
        {
            return "i16";
        }
        else if (&type == &natives.Int32)
        {
            return "i32";
        }
        else if (&type == &natives.Int64)
        {
            return "i64";
        }
        else if (&type == &natives.UInt8)
        {
            return "u8";
        }
        else if (&type == &natives.UInt16)
        {
            return "u16";
        }
        else if (&type == &natives.UInt32)
        {
            return "u32";
        }
        else if (&type == &natives.UInt64)
        {
            return "u64";
        }
        else if (&type == &natives.Int)
        {
            return "int";
        }
        else if (&type == &natives.Float32)
        {
            return "f32";
        }
        else if (&type == &natives.Float64)
        {
            return "f64";
        }

        ACE_UNREACHABLE();
    }

    static auto CreateName(std::string mainName) -> std::vector<std::string>
    {
        return { Std::GetName(), mainName };
    }

    static auto CreateFromName(
        const Natives& natives,
        const NativeType& targetType,
        const NativeType& fromType
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, targetType) +
            "_from_" +
            CreateTypeAliasNameString(natives, fromType)
        );
    }

    static auto CreateUnaryNegationName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_unary_negation"
        );
    }

    static auto CreateNOTName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_NOT"
        );
    }

    static auto CreateMultiplicationName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_multiplication"
        );
    }

    static auto CreateDivisionName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_division"
        );
    }

    static auto CreateRemainderName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_remainder"
        );
    }

    static auto CreateAdditionName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_addition"
        );
    }

    static auto CreateSubtractionName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_subtraction"
        );
    }

    static auto CreateRightShiftName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_right_shift"
        );
    }

    static auto CreateLeftShiftName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_left_shift"
        );
    }

    static auto CreateLessThanName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_less_than"
        );
    }

    static auto CreateGreaterThanName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_greater_than"
        );
    }

    static auto CreateLessThanEqualsName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_less_than_equals"
        );
    }

    static auto CreateGreaterThanEqualsName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_greater_than_equals"
        );
    }

    static auto CreateEqualsName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(CreateTypeAliasNameString(natives, type) + "_equals");
    }

    static auto CreateNotEqualsName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(
            CreateTypeAliasNameString(natives, type) + "_not_equals"
        );
    }

    static auto CreateANDName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(CreateTypeAliasNameString(natives, type) + "_AND");
    }

    static auto CreateXORName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(CreateTypeAliasNameString(natives, type) + "_XOR");
    }

    static auto CreateORName(
        const Natives& natives,
        const NativeType& type
    ) -> std::vector<std::string>
    {
        return CreateName(CreateTypeAliasNameString(natives, type) + "_OR");
    }

    namespace I
    {
        static auto FromInt(
            const Natives& natives,
            const NativeType& targetType,
            const NativeType& fromType
        ) -> NativeFunction
        {
            auto* const compilation = fromType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const fromIRType =
                    fromType.GetIRType(emitter.GetContext());
                auto* const toIRType =
                    targetType.GetIRType(emitter.GetContext());

                const bool isSigned = natives.IsIntTypeSigned(fromType);

                auto* const value = isSigned ?
                    emitter.GetBlock().Builder.CreateSExtOrTrunc(
                        emitter.EmitLoadArg(0, fromIRType),
                        toIRType
                    ) :
                    emitter.GetBlock().Builder.CreateZExtOrTrunc(
                        emitter.EmitLoadArg(0, fromIRType),
                        toIRType
                    );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateFromName(natives, targetType, fromType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto FromFloat(
            const Natives& natives,
            const NativeType& targetType,
            const NativeType& fromType
        ) -> NativeFunction
        {
            auto* const compilation = fromType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const fromIRType =
                    fromType.GetIRType(emitter.GetContext());
                auto* const toIRType =
                    targetType.GetIRType(emitter.GetContext());

                const bool isSigned = natives.IsIntTypeSigned(targetType);

                auto* const value = isSigned ?
                    emitter.GetBlock().Builder.CreateFPToSI(
                        emitter.EmitLoadArg(0, fromIRType),
                        toIRType
                    ) :
                    emitter.GetBlock().Builder.CreateFPToUI(
                        emitter.EmitLoadArg(0, fromIRType),
                        toIRType
                    );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateFromName(natives, targetType, fromType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto UnaryNegation(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateMul(
                    emitter.EmitLoadArg(0, selfIRType),
                    llvm::ConstantInt::get(selfIRType, -1)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateUnaryNegationName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto NOT(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateXor(
                    emitter.EmitLoadArg(0, selfIRType),
                    llvm::ConstantInt::get(selfIRType, -1)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateNOTName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Multiplication(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateMul(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateMultiplicationName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Division(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());
                
                const bool isSigned = natives.IsIntTypeSigned(selfType);

                auto* const value = isSigned ?
                    emitter.GetBlock().Builder.CreateSDiv(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    ) :
                    emitter.GetBlock().Builder.CreateUDiv(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateDivisionName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Remainder(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                const bool isSigned = natives.IsIntTypeSigned(selfType);

                auto* const value = isSigned ?
                    emitter.GetBlock().Builder.CreateSRem(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    ) :
                    emitter.GetBlock().Builder.CreateURem(
                        emitter.EmitLoadArg(0, selfIRType),
                        emitter.EmitLoadArg(1, selfIRType)
                    );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateRemainderName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Addition(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateAdd(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateAdditionName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Subtraction(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateSub(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateSubtractionName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto RightShift(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateAShr(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateRightShiftName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        };

        static auto LeftShift(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateShl(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateLeftShiftName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto LessThan(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateCmp(
                    llvm::CmpInst::Predicate::ICMP_SLT,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateLessThanName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto GreaterThan(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateCmp(
                    llvm::CmpInst::Predicate::ICMP_SGT,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateGreaterThanName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto LessThanEquals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateCmp(
                    llvm::CmpInst::Predicate::ICMP_SLE,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateLessThanEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto GreaterThanEquals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateCmp(
                    llvm::CmpInst::Predicate::ICMP_SGE,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateGreaterThanEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Equals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateCmp(
                    llvm::CmpInst::Predicate::ICMP_EQ,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto NotEquals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateCmp(
                    llvm::CmpInst::Predicate::ICMP_NE,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateNotEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto AND(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateAnd(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateANDName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto XOR(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateXor(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateXORName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto OR(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateOr(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateORName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }
    }

    namespace F
    {
        static auto FromInt(
            const Natives& natives,
            const NativeType& targetType,
            const NativeType& fromType
        ) -> NativeFunction
        {
            auto* const compilation = fromType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const fromIRType =
                    fromType.GetIRType(emitter.GetContext());
                auto* const toIRType =
                    targetType.GetIRType(emitter.GetContext());

                const bool isSigned = natives.IsIntTypeSigned(fromType);

                auto* const value = isSigned ? 
                    emitter.GetBlock().Builder.CreateSIToFP(
                        emitter.EmitLoadArg(0, fromIRType),
                        toIRType
                    ) :
                    emitter.GetBlock().Builder.CreateUIToFP(
                        emitter.EmitLoadArg(0, fromIRType),
                        toIRType
                    );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return
            {
                compilation,
                CreateFromName(natives, targetType, fromType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto UnaryNegation(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFMul(
                    emitter.EmitLoadArg(0, selfIRType),
                    llvm::ConstantFP::get(selfIRType, -1)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateUnaryNegationName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Multiplication(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFMul(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateMultiplicationName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Division(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFDiv(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );
        
                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateDivisionName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Remainder(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFRem(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateRemainderName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Addition(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFAdd(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateAdditionName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Subtraction(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFSub(
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateSubtractionName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto LessThan(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFCmp(
                    llvm::CmpInst::Predicate::FCMP_OLT,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateLessThanName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto GreaterThan(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFCmp(
                    llvm::CmpInst::Predicate::FCMP_OGT,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateGreaterThanName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto LessThanEquals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFCmp(
                    llvm::CmpInst::Predicate::FCMP_OLE,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateLessThanEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto GreaterThanEquals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFCmp(
                    llvm::CmpInst::Predicate::FCMP_OGE,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateGreaterThanEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto Equals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFCmp(
                    llvm::CmpInst::Predicate::FCMP_OEQ,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }

        static auto NotEquals(
            const Natives& natives,
            const NativeType& selfType
        ) -> NativeFunction
        {
            auto* const compilation = selfType.GetCompilation();

            auto blockEmitter = [&](Emitter& emitter)
            {
                auto* const selfIRType =
                    selfType.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFCmp(
                    llvm::CmpInst::Predicate::FCMP_ONE,
                    emitter.EmitLoadArg(0, selfIRType),
                    emitter.EmitLoadArg(1, selfIRType)
                );

                emitter.GetBlock().Builder.CreateRet(value);
            };

            return NativeFunction
            {
                compilation,
                CreateNotEqualsName(natives, selfType),
                NativeSymbolKind::Concrete,
                std::move(blockEmitter),
            };
        }
    }

    Natives::Natives(
        Compilation* const compilation
    ) : Int8
        {
            compilation,
            std::vector{ Std::GetName().data(), "Int8" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt8Ty(context);
            },
        },
        Int16
        {
            compilation,
            std::vector{ Std::GetName().data(), "Int16" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt16Ty(context);
            },
        },
        Int32
        {
            compilation,
            std::vector{ Std::GetName().data(), "Int32" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(context);
            },
        },
        Int64
        {
            compilation,
            std::vector{ Std::GetName().data(), "Int64" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt64Ty(context);
            },
        },

        UInt8
        {
            compilation,
            std::vector{ Std::GetName().data(), "UInt8" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt8Ty(context);
            },
        },
        UInt16
        {
            compilation,
            std::vector{ Std::GetName().data(), "UInt16" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt16Ty(context);
            },
        },
        UInt32
        {
            compilation,
            std::vector{ Std::GetName().data(), "UInt32" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(context);
            },
        },
        UInt64
        {
            compilation,
            std::vector{ Std::GetName().data(), "UInt64" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt64Ty(context);
            },
        },

        Int
        {
            compilation,
            std::vector{ Std::GetName().data(), "Int" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt32Ty(context);
            },
        },

        Float32
        {
            compilation,
            std::vector{ Std::GetName().data(), "Float32" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getFloatTy(context);
            },
        },
        Float64
        {
            compilation,
            std::vector{ Std::GetName().data(), "Float64" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getDoubleTy(context);
            },
        },

        Bool
        {
            compilation,
            std::vector{ Std::GetName().data(), "Bool" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt1Ty(context);
            },
        },
        String
        {
            compilation,
            std::vector{ Std::GetName().data(), "String" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::NonTrivial,
            {},
        },

        Ptr
        {
            compilation,
            std::vector{ Std::GetName().data(), "Ptr" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::Trivial,
            [compilation](llvm::LLVMContext& context) -> llvm::Type*
            {
                return llvm::Type::getInt8PtrTy(context);
            },
        },

        Ref
        {
            compilation,
            std::vector{ Std::GetName().data(), "Ref" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        WeakPtr
        {
            compilation,
            std::vector{ Std::GetName().data(), "rc", "WeakPtr" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        StrongPtr
        {
            compilation,
            std::vector{ Std::GetName().data(), "rc", "StrongPtr" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        DynStrongPtr
        {
            compilation,
            std::vector{ Std::GetName().data(), "rc", "DynStrongPtr" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        DynStrongPtrData
        {
            compilation,
            std::vector{ Std::GetName().data(), "rc", "DynStrongPtrData" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },

        Minus
        {
            compilation,
            std::vector{ Std::GetName().data(), "Minus" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        Multiply
        {
            compilation,
            std::vector{ Std::GetName().data(), "Multiply" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        Divide
        {
            compilation,
            std::vector{ Std::GetName().data(), "Divide" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        Remainder
        {
            compilation,
            std::vector{ Std::GetName().data(), "Remainder" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        Add
        {
            compilation,
            std::vector{ Std::GetName().data(), "Add" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        Subtract
        {
            compilation,
            std::vector{ Std::GetName().data(), "Subtract" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        Equal
        {
            compilation,
            std::vector{ Std::GetName().data(), "Equal" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        AND
        {
            compilation,
            std::vector{ Std::GetName().data(), "AND" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        OR
        {
            compilation,
            std::vector{ Std::GetName().data(), "OR" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        XOR
        {
            compilation,
            std::vector{ Std::GetName().data(), "XOR" },
            NativeSymbolKind::Root,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },
        Lifetime
        {
            compilation,
            std::vector{ Std::GetName().data(), "Lifetime" },
            NativeSymbolKind::Concrete,
            NativeCopyabilityKind::NonTrivial,
            std::nullopt,
        },

        print_int
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "print_int" },
            NativeSymbolKind::Concrete,
            [this, compilation](Emitter& emitter)
            {
                auto* const compilation = emitter.GetCompilation();

                auto* const intType = emitter.GetType(
                    compilation->GetNatives().Int.GetSymbol()
                );

                emitter.EmitPrintf({
                    emitter.EmitString("%" PRId32 "\n"),
                    emitter.EmitLoadArg(0, intType),
                });

                emitter.GetBlock().Builder.CreateRetVoid();
            },
        },
        print_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "print_ptr" },
            NativeSymbolKind::Concrete,
            [compilation](Emitter& emitter)
            {
                auto* const compilation = emitter.GetCompilation();

                emitter.EmitPrintf({
                    emitter.EmitString("0x%" PRIXPTR "\n"),
                    emitter.EmitLoadArg(0, emitter.GetPtrType()),
                });

                emitter.GetBlock().Builder.CreateRetVoid();
            }
        },

        alloc
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "mem", "alloc" },
            NativeSymbolKind::Concrete,
            [this](Emitter& emitter)
            {
                auto* const mallocFunction =
                    emitter.GetC().GetFunctions().GetMalloc();

                auto* const intTypeSymbol = Int.GetSymbol();
                auto* const intType = emitter.GetType(intTypeSymbol);

                auto* const cSizeType = mallocFunction->arg_begin()->getType();

                auto* const sizeValue = emitter.EmitLoadArg(0, intType);

                auto* const convertedSizeValue = emitter.GetBlock().Builder.CreateZExtOrTrunc(
                    sizeValue,
                    cSizeType
                );

                auto* const callInst = emitter.GetBlock().Builder.CreateCall(
                    mallocFunction,
                    { convertedSizeValue }
                );

                emitter.GetBlock().Builder.CreateRet(callInst);
            }
        },
        dealloc
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "mem", "dealloc" },
            NativeSymbolKind::Concrete,
            [this](Emitter& emitter)
            {
                auto* const freeFunction =
                    emitter.GetC().GetFunctions().GetFree();

                auto* const blockValue =
                    emitter.EmitLoadArg(0, emitter.GetPtrType());

                emitter.GetBlock().Builder.CreateCall(
                    freeFunction,
                    { blockValue }
                );

                emitter.GetBlock().Builder.CreateRetVoid();
            }
        },
        copy
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "mem", "copy" },
            NativeSymbolKind::Concrete,
            [this](Emitter& emitter)
            {
                auto* const memcpyFunction =
                    emitter.GetC().GetFunctions().GetMemcpy();

                auto* const intTypeSymbol = Int.GetSymbol();
                auto* const intType = emitter.GetType(intTypeSymbol);

                auto* const cSizeType =
                    (memcpyFunction->arg_begin() + 2)->getType();

                auto* const srcValue =
                    emitter.EmitLoadArg(0, emitter.GetPtrType());
                auto* const dstValue =
                    emitter.EmitLoadArg(1, emitter.GetPtrType());
                auto* const sizeValue = emitter.EmitLoadArg(2, intType);

                auto* const convertedSizeValue = emitter.GetBlock().Builder.CreateZExtOrTrunc(
                    sizeValue,
                    cSizeType
                );

                emitter.GetBlock().Builder.CreateCall(
                    memcpyFunction,
                    { dstValue, srcValue, convertedSizeValue }
                );

                emitter.GetBlock().Builder.CreateRetVoid();
            }
        },

        lookup_vtbl_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "lookup_vtbl_ptr" },
            NativeSymbolKind::Concrete,
            [this](Emitter& emitter)
            {
                auto* const typeInfoPtr =
                    emitter.EmitLoadArg(0, emitter.GetPtrType());
                auto* const traitTypeInfoPtr =
                    emitter.EmitLoadArg(1, emitter.GetPtrType());

                auto* const typeInfoPtrPtr = emitter.GetBlock().Builder.CreateAlloca(
                    emitter.GetPtrType()
                );
                emitter.GetBlock().Builder.CreateStore(
                    typeInfoPtr,
                    typeInfoPtrPtr
                );

                auto* const traitTypeInfoPtrPtr = emitter.GetBlock().Builder.CreateAlloca(
                    emitter.GetPtrType()
                );
                emitter.GetBlock().Builder.CreateStore(
                    traitTypeInfoPtr,
                    traitTypeInfoPtrPtr
                );

                auto* const typeVtblPtr = emitter.EmitCall(
                    sublookup_vtbl_ptr.GetSymbol(),
                    { typeInfoPtrPtr, traitTypeInfoPtrPtr }
                );

                auto* const condition = emitter.GetBlock().Builder.CreateICmpEQ(
                    typeVtblPtr,
                    llvm::ConstantPointerNull::get(emitter.GetPtrType())
                );
                auto nullBlock = std::make_unique<EmittingBlock>(
                    emitter.GetContext(),
                    emitter.GetFunction()
                );
                auto nonNullBlock = std::make_unique<EmittingBlock>(
                    emitter.GetContext(),
                    emitter.GetFunction()
                );
                emitter.GetBlock().Builder.CreateCondBr(
                    condition,
                    nullBlock->Block,
                    nonNullBlock->Block
                );

                emitter.SetBlock(std::move(nonNullBlock));
                emitter.GetBlock().Builder.CreateRet(typeVtblPtr);

                emitter.SetBlock(std::move(nullBlock));

                auto* const traitTypeVtblPtr = emitter.EmitCall(
                    sublookup_vtbl_ptr.GetSymbol(),
                    { traitTypeInfoPtrPtr, typeInfoPtrPtr }
                );

                emitter.GetBlock().Builder.CreateRet(traitTypeVtblPtr);
            }       
        },
        sublookup_vtbl_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "sublookup_vtbl_ptr" },
            NativeSymbolKind::Concrete,
            [this](Emitter& emitter)
            {
                auto* const typeInfoPtr =
                    emitter.EmitLoadArg(0, emitter.GetPtrType());
                auto* const targetTypeInfoPtr =
                    emitter.EmitLoadArg(1, emitter.GetPtrType());

                auto* const intType = emitter.GetType(Int.GetSymbol());
                auto* const arrayType =
                    llvm::ArrayType::get(emitter.GetPtrType(), 0);

                auto* const i = emitter.GetBlock().Builder.CreateAlloca(
                    intType,
                    nullptr,
                    "i"
                );
                emitter.GetBlock().Builder.CreateStore(
                    llvm::ConstantInt::get(intType, 0),
                    i
                );

                auto* const arrayPtr = emitter.GetBlock().Builder.CreateStructGEP(
                    emitter.GetTypeInfoType(),
                    typeInfoPtr,
                    2
                );

                auto beginBlock = std::make_unique<EmittingBlock>(
                    emitter.GetContext(),
                    emitter.GetFunction()
                );
                auto matchingBlock = std::make_unique<EmittingBlock>(
                    emitter.GetContext(),
                    emitter.GetFunction()
                );
                auto continueBlock = std::make_unique<EmittingBlock>(
                    emitter.GetContext(),
                    emitter.GetFunction()
                );
                auto endBlock = std::make_unique<EmittingBlock>(
                    emitter.GetContext(),
                    emitter.GetFunction()
                );

                auto* const rawBeginBlock    =    beginBlock->Block;
                auto* const rawMatchingBlock = matchingBlock->Block;
                auto* const rawContinueBlock = continueBlock->Block;
                auto* const rawEndBlock      =      endBlock->Block;

                emitter.GetBlock().Builder.CreateBr(continueBlock->Block);

                emitter.SetBlock(std::move(continueBlock));

                auto* const countPtr = emitter.GetBlock().Builder.CreateStructGEP(
                    emitter.GetTypeInfoType(),
                    typeInfoPtr,
                    1
                );

                auto* const continueCondition = emitter.GetBlock().Builder.CreateICmpSLT(
                    emitter.GetBlock().Builder.CreateLoad(intType, i),
                    emitter.GetBlock().Builder.CreateLoad(intType, countPtr)
                );

                emitter.GetBlock().Builder.CreateCondBr(
                    continueCondition,
                    rawBeginBlock,
                    rawEndBlock
                );

                emitter.SetBlock(std::move(beginBlock));

                auto* const typeInfoIndex = emitter.GetBlock().Builder.CreateMul(
                    emitter.GetBlock().Builder.CreateLoad(intType, i),
                    llvm::ConstantInt::get(intType, 2)
                );
                auto* const otherTypeInfoPtrPtr = emitter.GetBlock().Builder.CreateGEP(
                    arrayType,
                    arrayPtr,
                    { llvm::ConstantInt::get(intType, 0), typeInfoIndex }
                );
                auto* const otherTypeInfoPtr = emitter.GetBlock().Builder.CreateLoad(
                    emitter.GetPtrType(),
                    otherTypeInfoPtrPtr
                );

                auto* const a = emitter.GetBlock().Builder.CreateAlloca(
                    emitter.GetPtrType()
                );
                emitter.GetBlock().Builder.CreateStore(targetTypeInfoPtr, a);

                auto* const matchingCondition = emitter.GetBlock().Builder.CreateICmpEQ(
                    otherTypeInfoPtr,
                    targetTypeInfoPtr
                );

                emitter.GetBlock().Builder.CreateStore(
                    emitter.GetBlock().Builder.CreateAdd(
                        emitter.GetBlock().Builder.CreateLoad(intType, i),
                        llvm::ConstantInt::get(intType, 1)
                    ),
                    i
                );

                emitter.GetBlock().Builder.CreateCondBr(
                    matchingCondition,
                    rawMatchingBlock,
                    rawContinueBlock
                );

                emitter.SetBlock(std::move(matchingBlock));

                auto* const vtblIndex = emitter.GetBlock().Builder.CreateAdd(
                    typeInfoIndex,
                    llvm::ConstantInt::get(intType, 1)
                );
                auto* const vtblPtrPtr = emitter.GetBlock().Builder.CreateGEP(
                    arrayType,
                    arrayPtr,
                    { llvm::ConstantInt::get(intType, 0), vtblIndex }
                );
                auto* const vtblPtr = emitter.GetBlock().Builder.CreateLoad(
                    emitter.GetPtrType(),
                    vtblPtrPtr
                );
                emitter.GetBlock().Builder.CreateRet(vtblPtr);

                emitter.SetBlock(std::move(endBlock));

                emitter.GetBlock().Builder.CreateRet(
                    llvm::ConstantPointerNull::get(emitter.GetPtrType())
                );
            }
        },
        dyn_drop
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_drop" },
            NativeSymbolKind::Concrete,
            [this](Emitter& emitter)
            {
                auto* const valuePtr =
                    emitter.EmitLoadArg(0, emitter.GetPtrType());
                auto* const typeInfoPtr =
                    emitter.EmitLoadArg(1, emitter.GetPtrType());

                auto* const valuePtrPtr = emitter.GetBlock().Builder.CreateAlloca(
                    emitter.GetPtrType()
                );
                emitter.GetBlock().Builder.CreateStore(valuePtr, valuePtrPtr);

                auto* const dropGluePtrPtr = emitter.GetBlock().Builder.CreateStructGEP(
                    emitter.GetTypeInfoType(),
                    typeInfoPtr,
                    0
                );
                auto* const dropGluePtr = emitter.GetBlock().Builder.CreateLoad(
                    emitter.GetPtrType(),
                    dropGluePtrPtr
                );

                emitter.GetBlock().Builder.CreateCall(
                    emitter.GetDropGlueType(),
                    dropGluePtr,
                    { valuePtrPtr }
                );

                emitter.GetBlock().Builder.CreateRetVoid();
            }
        },

        i8_from_i16{ I::FromInt(*this, Int8, Int16) },
        i8_from_i32{ I::FromInt(*this, Int8, Int32) },
        i8_from_i64{ I::FromInt(*this, Int8, Int64) },
        i8_from_u8{ I::FromInt(*this, Int8, UInt8) },
        i8_from_u16{ I::FromInt(*this, Int8, UInt16) },
        i8_from_u32{ I::FromInt(*this, Int8, UInt32) },
        i8_from_u64{ I::FromInt(*this, Int8, UInt64) },
        i8_from_int{ I::FromInt(*this, Int8, Int) },
        i8_from_f32{ I::FromFloat(*this, Int8, Float32) },
        i8_from_f64{ I::FromFloat(*this, Int8, Float64) },
        i8_unary_negation{ I::UnaryNegation(*this, Int8) },
        i8_NOT{ I::NOT(*this, Int8) },
        i8_multiplication{ I::Multiplication(*this, Int8) },
        i8_division{ I::Division(*this, Int8) },
        i8_remainder{ I::Remainder(*this, Int8) },
        i8_addition{ I::Addition(*this, Int8) },
        i8_subtraction{ I::Subtraction(*this, Int8) },
        i8_right_shift{ I::RightShift(*this, Int8) },
        i8_left_shift{ I::LeftShift(*this, Int8) },
        i8_less_than{ I::LessThan(*this, Int8) },
        i8_greater_than{ I::GreaterThan(*this, Int8) },
        i8_less_than_equals{ I::LessThanEquals(*this, Int8) },
        i8_greater_than_equals{ I::GreaterThanEquals(*this, Int8) },
        i8_equals{ I::Equals(*this, Int8) },
        i8_not_equals{ I::NotEquals(*this, Int8) },
        i8_AND{ I::AND(*this, Int8) },
        i8_XOR{ I::XOR(*this, Int8) },
        i8_OR{ I::OR(*this, Int8) },

        i16_from_i8{ I::FromInt(*this, Int16, Int8) },
        i16_from_i32{ I::FromInt(*this, Int16, Int32) },
        i16_from_i64{ I::FromInt(*this, Int16, Int64) },
        i16_from_u8{ I::FromInt(*this, Int16, UInt8) },
        i16_from_u16{ I::FromInt(*this, Int16, UInt16) },
        i16_from_u32{ I::FromInt(*this, Int16, UInt32) },
        i16_from_u64{ I::FromInt(*this, Int16, UInt64) },
        i16_from_int{ I::FromInt(*this, Int16, Int) },
        i16_from_f32{ I::FromFloat(*this, Int16, Float32) },
        i16_from_f64{ I::FromFloat(*this, Int16, Float64) },
        i16_unary_negation{ I::UnaryNegation(*this, Int16) },
        i16_NOT{ I::NOT(*this, Int16) },
        i16_multiplication{ I::Multiplication(*this, Int16) },
        i16_division{ I::Division(*this, Int16) },
        i16_remainder{ I::Remainder(*this, Int16) },
        i16_addition{ I::Addition(*this, Int16) },
        i16_subtraction{ I::Subtraction(*this, Int16) },
        i16_right_shift{ I::RightShift(*this, Int16) },
        i16_left_shift{ I::LeftShift(*this, Int16) },
        i16_less_than{ I::LessThan(*this, Int16) },
        i16_greater_than{ I::GreaterThan(*this, Int16) },
        i16_less_than_equals{ I::LessThanEquals(*this, Int16) },
        i16_greater_than_equals{ I::GreaterThanEquals(*this, Int16) },
        i16_equals{ I::Equals(*this, Int16) },
        i16_not_equals{ I::NotEquals(*this, Int16) },
        i16_AND{ I::AND(*this, Int16) },
        i16_XOR{ I::XOR(*this, Int16) },
        i16_OR{ I::OR(*this, Int16) },

        i32_from_i8{ I::FromInt(*this, Int32, Int8) },
        i32_from_i16{ I::FromInt(*this, Int32, Int16) },
        i32_from_i64{ I::FromInt(*this, Int32, Int64) },
        i32_from_u8{ I::FromInt(*this, Int32, UInt8) },
        i32_from_u16{ I::FromInt(*this, Int32, UInt16) },
        i32_from_u32{ I::FromInt(*this, Int32, UInt32) },
        i32_from_u64{ I::FromInt(*this, Int32, UInt64) },
        i32_from_int{ I::FromInt(*this, Int32, Int) },
        i32_from_f32{ I::FromFloat(*this, Int32, Float32) },
        i32_from_f64{ I::FromFloat(*this, Int32, Float64) },
        i32_unary_negation{ I::UnaryNegation(*this, Int32) },
        i32_NOT{ I::NOT(*this, Int32) },
        i32_multiplication{ I::Multiplication(*this, Int32) },
        i32_division{ I::Division(*this, Int32) },
        i32_remainder{ I::Remainder(*this, Int32) },
        i32_addition{ I::Addition(*this, Int32) },
        i32_subtraction{ I::Subtraction(*this, Int32) },
        i32_right_shift{ I::RightShift(*this, Int32) },
        i32_left_shift{ I::LeftShift(*this, Int32) },
        i32_less_than{ I::LessThan(*this, Int32) },
        i32_greater_than{ I::GreaterThan(*this, Int32) },
        i32_less_than_equals{ I::LessThanEquals(*this, Int32) },
        i32_greater_than_equals{ I::GreaterThanEquals(*this, Int32) },
        i32_equals{ I::Equals(*this, Int32) },
        i32_not_equals{ I::NotEquals(*this, Int32) },
        i32_AND{ I::AND(*this, Int32) },
        i32_XOR{ I::XOR(*this, Int32) },
        i32_OR{ I::OR(*this, Int32) },

        i64_from_i8{ I::FromInt(*this, Int64, Int8) },
        i64_from_i16{ I::FromInt(*this, Int64, Int16) },
        i64_from_i32{ I::FromInt(*this, Int64, Int32) },
        i64_from_u8{ I::FromInt(*this, Int64, UInt8) },
        i64_from_u16{ I::FromInt(*this, Int64, UInt16) },
        i64_from_u32{ I::FromInt(*this, Int64, UInt32) },
        i64_from_u64{ I::FromInt(*this, Int64, UInt64) },
        i64_from_int{ I::FromInt(*this, Int64, Int) },
        i64_from_f32{ I::FromFloat(*this, Int64, Float32) },
        i64_from_f64{ I::FromFloat(*this, Int64, Float64) },
        i64_unary_negation{ I::UnaryNegation(*this, Int64) },
        i64_NOT{ I::NOT(*this, Int64) },
        i64_multiplication{ I::Multiplication(*this, Int64) },
        i64_division{ I::Division(*this, Int64) },
        i64_remainder{ I::Remainder(*this, Int64) },
        i64_addition{ I::Addition(*this, Int64) },
        i64_subtraction{ I::Subtraction(*this, Int64) },
        i64_right_shift{ I::RightShift(*this, Int64) },
        i64_left_shift{ I::LeftShift(*this, Int64) },
        i64_less_than{ I::LessThan(*this, Int64) },
        i64_greater_than{ I::GreaterThan(*this, Int64) },
        i64_less_than_equals{ I::LessThanEquals(*this, Int64) },
        i64_greater_than_equals{ I::GreaterThanEquals(*this, Int64) },
        i64_equals{ I::Equals(*this, Int64) },
        i64_not_equals{ I::NotEquals(*this, Int64) },
        i64_AND{ I::AND(*this, Int64) },
        i64_XOR{ I::XOR(*this, Int64) },
        i64_OR{ I::OR(*this, Int64) },

        u8_from_i8{ I::FromInt(*this, UInt8, Int8) },
        u8_from_i16{ I::FromInt(*this, UInt8, Int16) },
        u8_from_i32{ I::FromInt(*this, UInt8, Int32) },
        u8_from_i64{ I::FromInt(*this, UInt8, Int64) },
        u8_from_u16{ I::FromInt(*this, UInt8, UInt16) },
        u8_from_u32{ I::FromInt(*this, UInt8, UInt32) },
        u8_from_u64{ I::FromInt(*this, UInt8, UInt64) },
        u8_from_int{ I::FromInt(*this, UInt8, Int) },
        u8_from_f32{ I::FromFloat(*this, UInt8, Float32) },
        u8_from_f64{ I::FromFloat(*this, UInt8, Float64) },
        u8_unary_negation{ I::UnaryNegation(*this, UInt8) },
        u8_NOT{ I::NOT(*this, UInt8) },
        u8_multiplication{ I::Multiplication(*this, UInt8) },
        u8_division{ I::Division(*this, UInt8) },
        u8_remainder{ I::Remainder(*this, UInt8) },
        u8_addition{ I::Addition(*this, UInt8) },
        u8_subtraction{ I::Subtraction(*this, UInt8) },
        u8_right_shift{ I::RightShift(*this, UInt8) },
        u8_left_shift{ I::LeftShift(*this, UInt8) },
        u8_less_than{ I::LessThan(*this, UInt8) },
        u8_greater_than{ I::GreaterThan(*this, UInt8) },
        u8_less_than_equals{ I::LessThanEquals(*this, UInt8) },
        u8_greater_than_equals{ I::GreaterThanEquals(*this, UInt8) },
        u8_equals{ I::Equals(*this, UInt8) },
        u8_not_equals{ I::NotEquals(*this, UInt8) },
        u8_AND{ I::AND(*this, UInt8) },
        u8_XOR{ I::XOR(*this, UInt8) },
        u8_OR{ I::OR(*this, UInt8) },

        u16_from_i8{ I::FromInt(*this, UInt16, Int8) },
        u16_from_i16{ I::FromInt(*this, UInt16, Int16) },
        u16_from_i32{ I::FromInt(*this, UInt16, Int32) },
        u16_from_i64{ I::FromInt(*this, UInt16, Int64) },
        u16_from_u8{ I::FromInt(*this, UInt16, UInt8) },
        u16_from_u32{ I::FromInt(*this, UInt16, UInt32) },
        u16_from_u64{ I::FromInt(*this, UInt16, UInt64) },
        u16_from_int{ I::FromInt(*this, UInt16, Int) },
        u16_from_f32{ I::FromFloat(*this, UInt16, Float32) },
        u16_from_f64{ I::FromFloat(*this, UInt16, Float64) },
        u16_unary_negation{ I::UnaryNegation(*this, UInt16) },
        u16_NOT{ I::NOT(*this, UInt16) },
        u16_multiplication{ I::Multiplication(*this, UInt16) },
        u16_division{ I::Division(*this, UInt16) },
        u16_remainder{ I::Remainder(*this, UInt16) },
        u16_addition{ I::Addition(*this, UInt16) },
        u16_subtraction{ I::Subtraction(*this, UInt16) },
        u16_right_shift{ I::RightShift(*this, UInt16) },
        u16_left_shift{ I::LeftShift(*this, UInt16) },
        u16_less_than{ I::LessThan(*this, UInt16) },
        u16_greater_than{ I::GreaterThan(*this, UInt16) },
        u16_less_than_equals{ I::LessThanEquals(*this, UInt16) },
        u16_greater_than_equals{ I::GreaterThanEquals(*this, UInt16) },
        u16_equals{ I::Equals(*this, UInt16) },
        u16_not_equals{ I::NotEquals(*this, UInt16) },
        u16_AND{ I::AND(*this, UInt16) },
        u16_XOR{ I::XOR(*this, UInt16) },
        u16_OR{ I::OR(*this, UInt16) },

        u32_from_i8{ I::FromInt(*this, UInt32, Int8) },
        u32_from_i16{ I::FromInt(*this, UInt32, Int16) },
        u32_from_i32{ I::FromInt(*this, UInt32, Int32) },
        u32_from_i64{ I::FromInt(*this, UInt32, Int64) },
        u32_from_u8{ I::FromInt(*this, UInt32, UInt8) },
        u32_from_u16{ I::FromInt(*this, UInt32, UInt16) },
        u32_from_u64{ I::FromInt(*this, UInt32, UInt64) },
        u32_from_int{ I::FromInt(*this, UInt32, Int) },
        u32_from_f32{ I::FromFloat(*this, UInt32, Float32) },
        u32_from_f64{ I::FromFloat(*this, UInt32, Float64) },
        u32_unary_negation{ I::UnaryNegation(*this, UInt32) },
        u32_NOT{ I::NOT(*this, UInt32) },
        u32_multiplication{ I::Multiplication(*this, UInt32) },
        u32_division{ I::Division(*this, UInt32) },
        u32_remainder{ I::Remainder(*this, UInt32) },
        u32_addition{ I::Addition(*this, UInt32) },
        u32_subtraction{ I::Subtraction(*this, UInt32) },
        u32_right_shift{ I::RightShift(*this, UInt32) },
        u32_left_shift{ I::LeftShift(*this, UInt32) },
        u32_less_than{ I::LessThan(*this, UInt32) },
        u32_greater_than{ I::GreaterThan(*this, UInt32) },
        u32_less_than_equals{ I::LessThanEquals(*this, UInt32) },
        u32_greater_than_equals{ I::GreaterThanEquals(*this, UInt32) },
        u32_equals{ I::Equals(*this, UInt32) },
        u32_not_equals{ I::NotEquals(*this, UInt32) },
        u32_AND{ I::AND(*this, UInt32) },
        u32_XOR{ I::XOR(*this, UInt32) },
        u32_OR{ I::OR(*this, UInt32) },

        u64_from_i8{ I::FromInt(*this, UInt64, Int8) },
        u64_from_i16{ I::FromInt(*this, UInt64, Int16) },
        u64_from_i32{ I::FromInt(*this, UInt64, Int32) },
        u64_from_i64{ I::FromInt(*this, UInt64, Int64) },
        u64_from_u8{ I::FromInt(*this, UInt64, UInt8) },
        u64_from_u16{ I::FromInt(*this, UInt64, UInt16) },
        u64_from_u32{ I::FromInt(*this, UInt64, UInt32) },
        u64_from_int{ I::FromInt(*this, UInt64, Int) },
        u64_from_f32{ I::FromFloat(*this, UInt64, Float32) },
        u64_from_f64{ I::FromFloat(*this, UInt64, Float64) },
        u64_unary_negation{ I::UnaryNegation(*this, UInt64) },
        u64_NOT{ I::NOT(*this, UInt64) },
        u64_multiplication{ I::Multiplication(*this, UInt64) },
        u64_division{ I::Division(*this, UInt64) },
        u64_remainder{ I::Remainder(*this, UInt64) },
        u64_addition{ I::Addition(*this, UInt64) },
        u64_subtraction{ I::Subtraction(*this, UInt64) },
        u64_right_shift{ I::RightShift(*this, UInt64) },
        u64_left_shift{ I::LeftShift(*this, UInt64) },
        u64_less_than{ I::LessThan(*this, UInt64) },
        u64_greater_than{ I::GreaterThan(*this, UInt64) },
        u64_less_than_equals{ I::LessThanEquals(*this, UInt64) },
        u64_greater_than_equals{ I::GreaterThanEquals(*this, UInt64) },
        u64_equals{ I::Equals(*this, UInt64) },
        u64_not_equals{ I::NotEquals(*this, UInt64) },
        u64_AND{ I::AND(*this, UInt64) },
        u64_XOR{ I::XOR(*this, UInt64) },
        u64_OR{ I::OR(*this, UInt64) },

        int_from_i8{ I::FromInt(*this, Int, Int8) },
        int_from_i16{ I::FromInt(*this, Int, Int16) },
        int_from_i32{ I::FromInt(*this, Int, Int32) },
        int_from_i64{ I::FromInt(*this, Int, Int64) },
        int_from_u8{ I::FromInt(*this, Int, UInt8) },
        int_from_u16{ I::FromInt(*this, Int, UInt16) },
        int_from_u32{ I::FromInt(*this, Int, UInt32) },
        int_from_u64{ I::FromInt(*this, Int, UInt64) },
        int_from_f32{ I::FromFloat(*this, Int, Float32) },
        int_from_f64{ I::FromFloat(*this, Int, Float64) },
        int_unary_negation{ I::UnaryNegation(*this, Int) },
        int_NOT{ I::NOT(*this, Int) },
        int_multiplication{ I::Multiplication(*this, Int) },
        int_division{ I::Division(*this, Int) },
        int_remainder{ I::Remainder(*this, Int) },
        int_addition{ I::Addition(*this, Int) },
        int_subtraction{ I::Subtraction(*this, Int) },
        int_right_shift{ I::RightShift(*this, Int) },
        int_left_shift{ I::LeftShift(*this, Int) },
        int_less_than{ I::LessThan(*this, Int) },
        int_greater_than{ I::GreaterThan(*this, Int) },
        int_less_than_equals{ I::LessThanEquals(*this, Int) },
        int_greater_than_equals{ I::GreaterThanEquals(*this, Int) },
        int_equals{ I::Equals(*this, Int) },
        int_not_equals{ I::NotEquals(*this, Int) },
        int_AND{ I::AND(*this, Int) },
        int_XOR{ I::XOR(*this, Int) },
        int_OR{ I::OR(*this, Int) },

        f32_from_i8{ F::FromInt(*this, Float32, Int8) },
        f32_from_i16{ F::FromInt(*this, Float32, Int16) },
        f32_from_i32{ F::FromInt(*this, Float32, Int32) },
        f32_from_i64{ F::FromInt(*this, Float32, Int64) },
        f32_from_u8{ F::FromInt(*this, Float32, UInt8) },
        f32_from_u16{ F::FromInt(*this, Float32, UInt16) },
        f32_from_u32{ F::FromInt(*this, Float32, UInt32) },
        f32_from_u64{ F::FromInt(*this, Float32, UInt64) },
        f32_from_int{ F::FromInt(*this, Float32, Int) },
        f32_from_f64
        {
            compilation,
            CreateFromName(*this, Float32, Float64),
            NativeSymbolKind::Concrete,
            [this, compilation](Emitter& emitter)
            {
                auto* const float64IRType =
                    Float64.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFPTrunc(
                    emitter.EmitLoadArg(0, float64IRType),
                    llvm::Type::getFloatTy(emitter.GetContext())
                );

                emitter.GetBlock().Builder.CreateRet(value);
            }
        },
        f32_unary_negation{ F::UnaryNegation(*this, Float32) },
        f32_multiplication{ F::Multiplication(*this, Float32) },
        f32_division{ F::Division(*this, Float32) },
        f32_remainder{ F::Remainder(*this, Float32) },
        f32_addition{ F::Addition(*this, Float32) },
        f32_subtraction{ F::Subtraction(*this, Float32) },
        f32_less_than{ F::LessThan(*this, Float32) },
        f32_greater_than{ F::GreaterThan(*this, Float32) },
        f32_less_than_equals{ F::LessThanEquals(*this, Float32) },
        f32_greater_than_equals{ F::GreaterThanEquals(*this, Float32) },
        f32_equals{ F::Equals(*this, Float32) },
        f32_not_equals{ F::NotEquals(*this, Float32) },

        f64_from_i8{ F::FromInt(*this, Float64, Int8) },
        f64_from_i16{ F::FromInt(*this, Float64, Int16) },
        f64_from_i32{ F::FromInt(*this, Float64, Int32) },
        f64_from_i64{ F::FromInt(*this, Float64, Int64) },
        f64_from_u8{ F::FromInt(*this, Float64, UInt8) },
        f64_from_u16{ F::FromInt(*this, Float64, UInt16) },
        f64_from_u32{ F::FromInt(*this, Float64, UInt32) },
        f64_from_u64{ F::FromInt(*this, Float64, UInt64) },
        f64_from_int{ F::FromInt(*this, Float64, Int) },
        f64_from_f32
        {
            compilation,
            CreateFromName(*this, Float64, Float32),
            NativeSymbolKind::Concrete,
            [this, compilation](Emitter& emitter)
            {
                auto* const float32IRType =
                    Float32.GetIRType(emitter.GetContext());

                auto* const value = emitter.GetBlock().Builder.CreateFPExt(
                    emitter.EmitLoadArg(0, float32IRType),
                    llvm::Type::getDoubleTy(emitter.GetContext())
                );

                emitter.GetBlock().Builder.CreateRet(value);
            }
        },
        f64_unary_negation{ F::UnaryNegation(*this, Float64) },
        f64_multiplication{ F::Multiplication(*this, Float64) },
        f64_division{ F::Division(*this, Float64) },
        f64_remainder{ F::Remainder(*this, Float64) },
        f64_addition{ F::Addition(*this, Float64) },
        f64_subtraction{ F::Subtraction(*this, Float64) },
        f64_less_than{ F::LessThan(*this, Float64) },
        f64_greater_than{ F::GreaterThan(*this, Float64) },
        f64_less_than_equals{ F::LessThanEquals(*this, Float64) },
        f64_greater_than_equals{ F::GreaterThanEquals(*this, Float64) },
        f64_equals{ F::Equals(*this, Float64) },
        f64_not_equals{ F::NotEquals(*this, Float64) },

        weak_ptr_from
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "weak_ptr_from" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        weak_ptr_from_dyn
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "weak_ptr_from_dyn" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        weak_ptr_copy
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "weak_ptr_copy" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        weak_ptr_drop
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "weak_ptr_drop" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        weak_ptr_lock
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "weak_ptr_lock" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        weak_ptr_lock_dyn
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "weak_ptr_lock_dyn" },
            NativeSymbolKind::Root,
            std::nullopt,
        },

        strong_ptr_new
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "strong_ptr_new" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        strong_ptr_value
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "strong_ptr_value" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        strong_ptr_copy
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "strong_ptr_copy" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        strong_ptr_drop
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "strong_ptr_drop" },
            NativeSymbolKind::Root,
            std::nullopt,
        },

        dyn_strong_ptr_from
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_from" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_copy
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_copy" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_drop
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_drop" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_value_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_value_ptr" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_set_value_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_set_value_ptr" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_control_block_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_control_block_ptr" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_set_control_block_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_set_control_block_ptr" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_vtbl_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_vtbl_ptr" },
            NativeSymbolKind::Root,
            std::nullopt,
        },
        dyn_strong_ptr_set_vtbl_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "dyn_strong_ptr_set_vtbl_ptr" },
            NativeSymbolKind::Root,
            std::nullopt,
        },

        strong_ptr_to_dyn_strong_ptr
        {
            compilation,
            std::vector<std::string>{ Std::GetName(), "rc", "strong_ptr_to_dyn_strong_ptr" },
            NativeSymbolKind::Root,
            std::nullopt,
        },

        m_Natives
        {
            [this]()
            {
                std::vector<INative*> natives{};

                const auto types = m_Types.Get();
                natives.insert(end(natives), begin(types), end(types));

                const auto functions = m_Functions.Get();
                natives.insert(end(natives), begin(functions), end(functions));

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
                    &String,

                    &Ptr,

                    &Ref,
                    &WeakPtr,
                    &StrongPtr,
                    &DynStrongPtr,
                    &DynStrongPtrData,

                    &Minus,
                    &Multiply,
                    &Divide,
                    &Remainder,
                    &Add,
                    &Subtract,
                    &Equal,
                    &AND,
                    &OR,
                    &XOR,
                    &Lifetime,
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

                    &lookup_vtbl_ptr,
                    &sublookup_vtbl_ptr,
                    &dyn_drop,

                    &i8_from_i16,
                    &i8_from_i32,
                    &i8_from_i64,
                    &i8_from_u8,
                    &i8_from_u16,
                    &i8_from_u32,
                    &i8_from_u64,
                    &i8_from_int,
                    &i8_from_f32,
                    &i8_from_f64,
                    &i8_unary_negation,
                    &i8_NOT,
                    &i8_multiplication,
                    &i8_division,
                    &i8_remainder,
                    &i8_addition,
                    &i8_subtraction,
                    &i8_right_shift,
                    &i8_left_shift,
                    &i8_less_than,
                    &i8_greater_than,
                    &i8_less_than_equals,
                    &i8_greater_than_equals,
                    &i8_equals,
                    &i8_not_equals,
                    &i8_AND,
                    &i8_XOR,
                    &i8_OR,

                    &i16_from_i8,
                    &i16_from_i32,
                    &i16_from_i64,
                    &i16_from_u8,
                    &i16_from_u16,
                    &i16_from_u32,
                    &i16_from_u64,
                    &i16_from_int,
                    &i16_from_f32,
                    &i16_from_f64,
                    &i16_unary_negation,
                    &i16_NOT,
                    &i16_multiplication,
                    &i16_division,
                    &i16_remainder,
                    &i16_addition,
                    &i16_subtraction,
                    &i16_right_shift,
                    &i16_left_shift,
                    &i16_less_than,
                    &i16_greater_than,
                    &i16_less_than_equals,
                    &i16_greater_than_equals,
                    &i16_equals,
                    &i16_not_equals,
                    &i16_AND,
                    &i16_XOR,
                    &i16_OR,

                    &i32_from_i8,
                    &i32_from_i16,
                    &i32_from_i64,
                    &i32_from_u8,
                    &i32_from_u16,
                    &i32_from_u32,
                    &i32_from_u64,
                    &i32_from_int,
                    &i32_from_f32,
                    &i32_from_f64,
                    &i32_unary_negation,
                    &i32_NOT,
                    &i32_multiplication,
                    &i32_division,
                    &i32_remainder,
                    &i32_addition,
                    &i32_subtraction,
                    &i32_right_shift,
                    &i32_left_shift,
                    &i32_less_than,
                    &i32_greater_than,
                    &i32_less_than_equals,
                    &i32_greater_than_equals,
                    &i32_equals,
                    &i32_not_equals,
                    &i32_AND,
                    &i32_XOR,
                    &i32_OR,

                    &i64_from_i8,
                    &i64_from_i16,
                    &i64_from_i32,
                    &i64_from_u8,
                    &i64_from_u16,
                    &i64_from_u32,
                    &i64_from_u64,
                    &i64_from_int,
                    &i64_from_f32,
                    &i64_from_f64,
                    &i64_unary_negation,
                    &i64_NOT,
                    &i64_multiplication,
                    &i64_division,
                    &i64_remainder,
                    &i64_addition,
                    &i64_subtraction,
                    &i64_right_shift,
                    &i64_left_shift,
                    &i64_less_than,
                    &i64_greater_than,
                    &i64_less_than_equals,
                    &i64_greater_than_equals,
                    &i64_equals,
                    &i64_not_equals,
                    &i64_AND,
                    &i64_XOR,
                    &i64_OR,

                    &u8_from_i8,
                    &u8_from_i16,
                    &u8_from_i32,
                    &u8_from_i64,
                    &u8_from_u16,
                    &u8_from_u32,
                    &u8_from_u64,
                    &u8_from_int,
                    &u8_from_f32,
                    &u8_from_f64,
                    &u8_unary_negation,
                    &u8_NOT,
                    &u8_multiplication,
                    &u8_division,
                    &u8_remainder,
                    &u8_addition,
                    &u8_subtraction,
                    &u8_right_shift,
                    &u8_left_shift,
                    &u8_less_than,
                    &u8_greater_than,
                    &u8_less_than_equals,
                    &u8_greater_than_equals,
                    &u8_equals,
                    &u8_not_equals,
                    &u8_AND,
                    &u8_XOR,
                    &u8_OR,

                    &u16_from_i8,
                    &u16_from_i16,
                    &u16_from_i32,
                    &u16_from_i64,
                    &u16_from_u8,
                    &u16_from_u32,
                    &u16_from_u64,
                    &u16_from_int,
                    &u16_from_f32,
                    &u16_from_f64,
                    &u16_unary_negation,
                    &u16_NOT,
                    &u16_multiplication,
                    &u16_division,
                    &u16_remainder,
                    &u16_addition,
                    &u16_subtraction,
                    &u16_right_shift,
                    &u16_left_shift,
                    &u16_less_than,
                    &u16_greater_than,
                    &u16_less_than_equals,
                    &u16_greater_than_equals,
                    &u16_equals,
                    &u16_not_equals,
                    &u16_AND,
                    &u16_XOR,
                    &u16_OR,

                    &u32_from_i8,
                    &u32_from_i16,
                    &u32_from_i32,
                    &u32_from_i64,
                    &u32_from_u8,
                    &u32_from_u16,
                    &u32_from_u64,
                    &u32_from_int,
                    &u32_from_f32,
                    &u32_from_f64,
                    &u32_unary_negation,
                    &u32_NOT,
                    &u32_multiplication,
                    &u32_division,
                    &u32_remainder,
                    &u32_addition,
                    &u32_subtraction,
                    &u32_right_shift,
                    &u32_left_shift,
                    &u32_less_than,
                    &u32_greater_than,
                    &u32_less_than_equals,
                    &u32_greater_than_equals,
                    &u32_equals,
                    &u32_not_equals,
                    &u32_AND,
                    &u32_XOR,
                    &u32_OR,

                    &u64_from_i8,
                    &u64_from_i16,
                    &u64_from_i32,
                    &u64_from_i64,
                    &u64_from_u8,
                    &u64_from_u16,
                    &u64_from_u32,
                    &u64_from_int,
                    &u64_from_f32,
                    &u64_from_f64,
                    &u64_unary_negation,
                    &u64_NOT,
                    &u64_multiplication,
                    &u64_division,
                    &u64_remainder,
                    &u64_addition,
                    &u64_subtraction,
                    &u64_right_shift,
                    &u64_left_shift,
                    &u64_less_than,
                    &u64_greater_than,
                    &u64_less_than_equals,
                    &u64_greater_than_equals,
                    &u64_equals,
                    &u64_not_equals,
                    &u64_AND,
                    &u64_XOR,
                    &u64_OR,

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
                    &int_unary_negation,
                    &int_NOT,
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

                    &f32_from_i8,
                    &f32_from_i16,
                    &f32_from_i32,
                    &f32_from_i64,
                    &f32_from_u8,
                    &f32_from_u16,
                    &f32_from_u32,
                    &f32_from_u64,
                    &f32_from_int,
                    &f32_from_f64,
                    &f32_unary_negation,
                    &f32_multiplication,
                    &f32_division,
                    &f32_remainder,
                    &f32_addition,
                    &f32_subtraction,
                    &f32_less_than,
                    &f32_greater_than,
                    &f32_less_than_equals,
                    &f32_greater_than_equals,
                    &f32_equals,
                    &f32_not_equals,

                    &f64_from_i8,
                    &f64_from_i16,
                    &f64_from_i32,
                    &f64_from_i64,
                    &f64_from_u8,
                    &f64_from_u16,
                    &f64_from_u32,
                    &f64_from_u64,
                    &f64_from_int,
                    &f64_from_f32,
                    &f64_unary_negation,
                    &f64_multiplication,
                    &f64_division,
                    &f64_remainder,
                    &f64_addition,
                    &f64_subtraction,
                    &f64_less_than,
                    &f64_greater_than,
                    &f64_less_than_equals,
                    &f64_greater_than_equals,
                    &f64_equals,
                    &f64_not_equals,

                    &weak_ptr_from,
                    &weak_ptr_from_dyn,
                    &weak_ptr_copy,
                    &weak_ptr_drop,
                    &weak_ptr_lock,
                    &weak_ptr_lock_dyn,

                    &strong_ptr_new,
                    &strong_ptr_value,
                    &strong_ptr_copy,
                    &strong_ptr_drop,

                    &dyn_strong_ptr_from,
                    &dyn_strong_ptr_copy,
                    &dyn_strong_ptr_drop,
                    &dyn_strong_ptr_value_ptr,
                    &dyn_strong_ptr_set_value_ptr,
                    &dyn_strong_ptr_control_block_ptr,
                    &dyn_strong_ptr_set_control_block_ptr,
                    &dyn_strong_ptr_vtbl_ptr,
                    &dyn_strong_ptr_set_vtbl_ptr,

                    &strong_ptr_to_dyn_strong_ptr,
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
                    { Int8.GetSymbol(), i16_from_i8.GetSymbol() },
                    { UInt8.GetSymbol(), i16_from_u8.GetSymbol() },
                };
                map[Int32.GetSymbol()] = 
                {
                    { Int8.GetSymbol(), i32_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), i32_from_i16.GetSymbol() },
                    { UInt8.GetSymbol(), i32_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), i32_from_u16.GetSymbol() },
                };
                map[Int64.GetSymbol()] =
                {
                    { Int8.GetSymbol(), i64_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), i64_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), i64_from_i32.GetSymbol() },
                    { UInt8.GetSymbol(), i64_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), i64_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), i64_from_u32.GetSymbol() },
                };

                map[UInt8.GetSymbol()] =
                {
                };
                map[UInt16.GetSymbol()] =
                {
                    { UInt8.GetSymbol(), u16_from_u8.GetSymbol() },
                };
                map[UInt32.GetSymbol()] = 
                {
                    { UInt8.GetSymbol(), u32_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), u32_from_u16.GetSymbol() },
                };
                map[UInt64.GetSymbol()] =
                {
                    { UInt8.GetSymbol(), u64_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), u64_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), u64_from_u32.GetSymbol() },
                };
                map[Float32.GetSymbol()] =
                {
                    { Int8.GetSymbol(), f32_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), f32_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), f32_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), f32_from_i64.GetSymbol() },
                    { UInt8.GetSymbol(), f32_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), f32_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), f32_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), f32_from_u64.GetSymbol() },
                    { Int.GetSymbol(), f32_from_int.GetSymbol() },
                };
                map[Float64.GetSymbol()] =
                {
                    { Int8.GetSymbol(), f64_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), f64_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), f64_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), f64_from_i64.GetSymbol() },
                    { UInt8.GetSymbol(), f64_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), f64_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), f64_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), f64_from_u64.GetSymbol() },
                    { Int.GetSymbol(), f64_from_int.GetSymbol() },
                    { Float32.GetSymbol(), f64_from_f32.GetSymbol() },
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
                    { Int16.GetSymbol(), i8_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), i8_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), i8_from_i64.GetSymbol() },
                    { UInt8.GetSymbol(), i8_from_u8.GetSymbol() },
                    { UInt16.GetSymbol(), i8_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), i8_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), i8_from_u64.GetSymbol() },
                    { Int.GetSymbol(), i8_from_int.GetSymbol() },
                    { Float32.GetSymbol(), i8_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), i8_from_f64.GetSymbol() },
                };
                map[Int16.GetSymbol()] =
                {
                    { Int32.GetSymbol(), i16_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), i16_from_i64.GetSymbol() },
                    { UInt16.GetSymbol(), i16_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), i16_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), i16_from_u64.GetSymbol() },
                    { Int.GetSymbol(), i16_from_int.GetSymbol() },
                    { Float32.GetSymbol(), i16_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), i16_from_f64.GetSymbol() },
                };
                map[Int32.GetSymbol()] =
                {
                    { Int64.GetSymbol(), i32_from_i64.GetSymbol() },
                    { UInt32.GetSymbol(), i32_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), i32_from_u64.GetSymbol() },
                    { Int.GetSymbol(), i32_from_int.GetSymbol() },
                    { Float32.GetSymbol(), i32_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), i32_from_f64.GetSymbol() },
                };
                map[Int64.GetSymbol()] =
                {
                    { UInt64.GetSymbol(), i64_from_u64.GetSymbol() },
                    { Int.GetSymbol(), i64_from_int.GetSymbol() },
                    { Float32.GetSymbol(), i64_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), i64_from_f64.GetSymbol() },
                };

                map[UInt8.GetSymbol()] =
                {
                    { Int8.GetSymbol(), u8_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), u8_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), u8_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), u8_from_i64.GetSymbol() },
                    { UInt16.GetSymbol(), u8_from_u16.GetSymbol() },
                    { UInt32.GetSymbol(), u8_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), u8_from_u64.GetSymbol() },
                    { Int.GetSymbol(), u8_from_int.GetSymbol() },
                    { Float32.GetSymbol(), u8_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), u8_from_f64.GetSymbol() },
                };
                map[UInt16.GetSymbol()] =
                {
                    { Int8.GetSymbol(), u16_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), u16_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), u16_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), u16_from_i64.GetSymbol() },
                    { UInt32.GetSymbol(), u16_from_u32.GetSymbol() },
                    { UInt64.GetSymbol(), u16_from_u64.GetSymbol() },
                    { Int.GetSymbol(), u16_from_int.GetSymbol() },
                    { Float32.GetSymbol(), u16_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), u16_from_f64.GetSymbol() },
                };
                map[UInt32.GetSymbol()] =
                {
                    { Int8.GetSymbol(), u32_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), u32_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), u32_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), u32_from_i64.GetSymbol() },
                    { UInt64.GetSymbol(), u32_from_u64.GetSymbol() },
                    { Int.GetSymbol(), u32_from_int.GetSymbol() },
                    { Float32.GetSymbol(), u32_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), u32_from_f64.GetSymbol() },
                };
                map[UInt64.GetSymbol()] =
                {
                    { Int8.GetSymbol(), u64_from_i8.GetSymbol() },
                    { Int16.GetSymbol(), u64_from_i16.GetSymbol() },
                    { Int32.GetSymbol(), u64_from_i32.GetSymbol() },
                    { Int64.GetSymbol(), u64_from_i64.GetSymbol() },
                    { Int.GetSymbol(), u64_from_int.GetSymbol() },
                    { Float32.GetSymbol(), u64_from_f32.GetSymbol() },
                    { Float64.GetSymbol(), u64_from_f64.GetSymbol() },
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
                    { Float64.GetSymbol(), f32_from_f64.GetSymbol() },
                };
                map[Float64.GetSymbol()] =
                {
                };

                return map;
            }
        },

        m_UnaryOpMap
        {
            [this]()
            {
                std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>> map{};

                map[Int8.GetSymbol()] =
                {
                    { Op::UnaryNegation, i8_unary_negation.GetSymbol() },
                    { Op::NOT, i8_NOT.GetSymbol() },
                };
                map[Int16.GetSymbol()] =
                {
                    { Op::UnaryNegation, i16_unary_negation.GetSymbol() },
                    { Op::NOT, i16_NOT.GetSymbol() },
                };
                map[Int32.GetSymbol()] =
                {
                    { Op::UnaryNegation, i32_unary_negation.GetSymbol() },
                    { Op::NOT, i32_NOT.GetSymbol() },
                };
                map[Int64.GetSymbol()] =
                {
                    { Op::UnaryNegation, i64_unary_negation.GetSymbol() },
                    { Op::NOT, i64_NOT.GetSymbol() },
                };

                map[UInt8.GetSymbol()] =
                {
                    { Op::UnaryNegation, u8_unary_negation.GetSymbol() },
                    { Op::NOT, u8_NOT.GetSymbol() },
                };
                map[UInt16.GetSymbol()] =
                {
                    { Op::UnaryNegation, u16_unary_negation.GetSymbol() },
                    { Op::NOT, u16_NOT.GetSymbol() },
                };
                map[UInt32.GetSymbol()] =
                {
                    { Op::UnaryNegation, u32_unary_negation.GetSymbol() },
                    { Op::NOT, u32_NOT.GetSymbol() },
                };
                map[UInt64.GetSymbol()] =
                {
                    { Op::UnaryNegation, u64_unary_negation.GetSymbol() },
                    { Op::NOT, u64_NOT.GetSymbol() },
                };

                map[Int.GetSymbol()] =
                {
                    { Op::UnaryNegation, int_unary_negation.GetSymbol() },
                    { Op::NOT, int_NOT.GetSymbol() },
                };

                map[Float32.GetSymbol()] =
                {
                    { Op::UnaryNegation, f32_unary_negation.GetSymbol() },
                };
                map[Float64.GetSymbol()] =
                {
                    { Op::UnaryNegation, f64_unary_negation.GetSymbol() },
                };

                return map;
            }
        },
        m_BinaryOpMap
        {
            [this]()
            {
                std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>> map{};

                map[Int8.GetSymbol()] =
                {
                    { Op::RightShift, i8_right_shift.GetSymbol() },
                    { Op::LeftShift, i8_left_shift.GetSymbol() },
                    { Op::LessThan, i8_less_than.GetSymbol() },
                    { Op::GreaterThan, i8_greater_than.GetSymbol() },
                    { Op::LessThanEquals, i8_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, i8_greater_than_equals.GetSymbol() },
                };
                map[Int16.GetSymbol()] =
                {
                    { Op::RightShift, i16_right_shift.GetSymbol() },
                    { Op::LeftShift, i16_left_shift.GetSymbol() },
                    { Op::LessThan, i16_less_than.GetSymbol() },
                    { Op::GreaterThan, i16_greater_than.GetSymbol() },
                    { Op::LessThanEquals, i16_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, i16_greater_than_equals.GetSymbol() },
                };
                map[Int32.GetSymbol()] =
                {
                    { Op::RightShift, i32_right_shift.GetSymbol() },
                    { Op::LeftShift, i32_left_shift.GetSymbol() },
                    { Op::LessThan, i32_less_than.GetSymbol() },
                    { Op::GreaterThan, i32_greater_than.GetSymbol() },
                    { Op::LessThanEquals, i32_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, i32_greater_than_equals.GetSymbol() },
                };
                map[Int64.GetSymbol()] =
                {
                    { Op::RightShift, i64_right_shift.GetSymbol() },
                    { Op::LeftShift, i64_left_shift.GetSymbol() },
                    { Op::LessThan, i64_less_than.GetSymbol() },
                    { Op::GreaterThan, i64_greater_than.GetSymbol() },
                    { Op::LessThanEquals, i64_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, i64_greater_than_equals.GetSymbol() },
                };

                map[UInt8.GetSymbol()] =
                {
                    { Op::RightShift, u8_right_shift.GetSymbol() },
                    { Op::LeftShift, u8_left_shift.GetSymbol() },
                    { Op::LessThan, u8_less_than.GetSymbol() },
                    { Op::GreaterThan, u8_greater_than.GetSymbol() },
                    { Op::LessThanEquals, u8_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, u8_greater_than_equals.GetSymbol() },
                };
                map[UInt16.GetSymbol()] =
                {
                    { Op::RightShift, u16_right_shift.GetSymbol() },
                    { Op::LeftShift, u16_left_shift.GetSymbol() },
                    { Op::LessThan, u16_less_than.GetSymbol() },
                    { Op::GreaterThan, u16_greater_than.GetSymbol() },
                    { Op::LessThanEquals, u16_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, u16_greater_than_equals.GetSymbol() },
                };
                map[UInt32.GetSymbol()] =
                {
                    { Op::RightShift, u32_right_shift.GetSymbol() },
                    { Op::LeftShift, u32_left_shift.GetSymbol() },
                    { Op::LessThan, u32_less_than.GetSymbol() },
                    { Op::GreaterThan, u32_greater_than.GetSymbol() },
                    { Op::LessThanEquals, u32_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, u32_greater_than_equals.GetSymbol() },
                };
                map[UInt64.GetSymbol()] =
                {
                    { Op::RightShift, u64_right_shift.GetSymbol() },
                    { Op::LeftShift, u64_left_shift.GetSymbol() },
                    { Op::LessThan, u64_less_than.GetSymbol() },
                    { Op::GreaterThan, u64_greater_than.GetSymbol() },
                    { Op::LessThanEquals, u64_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, u64_greater_than_equals.GetSymbol() },
                };

                map[Int.GetSymbol()] =
                {
                    { Op::RightShift, int_right_shift.GetSymbol() },
                    { Op::LeftShift, int_left_shift.GetSymbol() },
                    { Op::LessThan, int_less_than.GetSymbol() },
                    { Op::GreaterThan, int_greater_than.GetSymbol() },
                    { Op::LessThanEquals, int_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, int_greater_than_equals.GetSymbol() },
                };

                map[Float32.GetSymbol()] =
                {
                    { Op::LessThan, f32_less_than.GetSymbol() },
                    { Op::GreaterThan, f32_greater_than.GetSymbol() },
                    { Op::LessThanEquals, f32_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, f32_greater_than_equals.GetSymbol() },
                };
                map[Float64.GetSymbol()] =
                {
                    { Op::LessThan, f64_less_than.GetSymbol() },
                    { Op::GreaterThan, f64_greater_than.GetSymbol() },
                    { Op::LessThanEquals, f64_less_than_equals.GetSymbol() },
                    { Op::GreaterThanEquals, f64_greater_than_equals.GetSymbol() },
                };

                return map;
            }
        },

        m_CopyOpMap
        {
            [this]()
            {
                return std::unordered_map<ITypeSymbol*, FunctionSymbol*>
                {
                    { WeakPtr.GetSymbol(), weak_ptr_copy.GetSymbol() },
                    { StrongPtr.GetSymbol(), strong_ptr_copy.GetSymbol() },
                    { DynStrongPtr.GetSymbol(), dyn_strong_ptr_copy.GetSymbol() },
                };
            }
        },
        m_DropOpMap
        {
            [this]()
            {
                return std::unordered_map<ITypeSymbol*, FunctionSymbol*>
                {
                    { WeakPtr.GetSymbol(), weak_ptr_drop.GetSymbol() },
                    { StrongPtr.GetSymbol(), strong_ptr_drop.GetSymbol() },
                    { DynStrongPtr.GetSymbol(), dyn_strong_ptr_drop.GetSymbol() },
                };
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

    auto Natives::GetUnaryOpMap()  const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>>&
    {
        return m_UnaryOpMap.Get();
    }

    auto Natives::GetBinaryOpMap() const -> const std::unordered_map<ITypeSymbol*, std::unordered_map<Op, FunctionSymbol*>>&
    {
        return m_BinaryOpMap.Get();
    }

    auto Natives::GetCopyOpMap() const -> const std::unordered_map<ITypeSymbol*, FunctionSymbol*>&
    {
        return m_CopyOpMap.Get();
    }

    auto Natives::GetDropOpMap() const -> const std::unordered_map<ITypeSymbol*, FunctionSymbol*>&
    {
        return m_DropOpMap.Get();
    }

    auto Natives::IsIntTypeSigned(const NativeType& intType) const -> bool
    {
        return m_SignedIntTypesSet.Get().contains(&intType);
    }
}
