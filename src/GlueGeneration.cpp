#include "GlueGeneration.hpp"

#include <memory>
#include <vector>

#include "Application.hpp"
#include "Compilation.hpp"
#include "Emittable.hpp"
#include "Emitter.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/EmittableTypeSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Symbols/Vars/FieldVarSymbol.hpp"
#include "AnonymousIdent.hpp"

namespace Ace::GlueGeneration
{
    static auto GetOrDeclareAndBindCopyGlueSymbols(
        Compilation* const compilation,
        IConcreteTypeSymbol* const typeSymbol
    ) -> FunctionSymbol*
    {
        if (typeSymbol->GetCopyGlue().has_value())
        {
            return typeSymbol->GetCopyGlue().value();
        }

        const auto scope     = typeSymbol->GetUnaliased()->GetScope();
        const auto bodyScope = scope->CreateChild();

        const std::string nameString = AnonymousIdent::Create(
            "copy_glue",
            typeSymbol->CreateLocalSignature()
        );
        const Ident name
        {
            typeSymbol->GetName().SrcLocation,
            nameString,
        };

        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            bodyScope,
            SymbolCategory::Static,
            AccessModifier::Pub,
            name,
            compilation->GetVoidTypeSymbol(),
            std::vector<ITypeSymbol*>{}
        );

        auto* const glueSymbol = DiagnosticBag::CreateNoError().Collect(
            Scope::DeclareSymbol(std::move(ownedGlueSymbol))
        );
        typeSymbol->BindCopyGlue(glueSymbol);

        const Ident selfName
        {
            typeSymbol->GetName().SrcLocation,
            AnonymousIdent::Create("self"),
        };
        (void)DiagnosticBag::CreateNoError().Collect(
            Scope::DeclareSymbol(std::make_unique<NormalParamVarSymbol>(
                bodyScope,
                selfName,
                dynamic_cast<ISizedTypeSymbol*>(typeSymbol->GetWithRef()),
                0
            ))
        );

        const Ident otherName
        {
            typeSymbol->GetName().SrcLocation,
            AnonymousIdent::Create("other"),
        };
        (void)DiagnosticBag::CreateNoError().Collect(
            Scope::DeclareSymbol(std::make_unique<NormalParamVarSymbol>(
                bodyScope,
                otherName,
                dynamic_cast<ISizedTypeSymbol*>(typeSymbol->GetWithRef()),
                1
            ))
        );

        return glueSymbol;
    }

    static auto GetOrDeclareAndBindDropGlueSymbols(
        Compilation* const compilation,
        IConcreteTypeSymbol* const typeSymbol
    ) -> FunctionSymbol*
    {
        if (typeSymbol->GetDropGlue().has_value())
        {
            return typeSymbol->GetDropGlue().value();
        }

        const auto scope     = typeSymbol->GetUnaliased()->GetScope();
        const auto bodyScope = scope->CreateChild();

        const std::string nameString = AnonymousIdent::Create(
            "drop_glue",
            typeSymbol->CreateLocalSignature()
        );
        const Ident name
        {
            typeSymbol->GetName().SrcLocation,
            nameString,
        };

        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            bodyScope,
            SymbolCategory::Static,
            AccessModifier::Pub,
            name,
            compilation->GetVoidTypeSymbol(),
            std::vector<ITypeSymbol*>{}
        );

        auto* const glueSymbol = DiagnosticBag::CreateNoError().Collect(
            Scope::DeclareSymbol(std::move(ownedGlueSymbol))
        );
        typeSymbol->BindDropGlue(glueSymbol);

        const Ident selfName
        {
            typeSymbol->GetName().SrcLocation,
            AnonymousIdent::Create("self"),
        };
        (void)DiagnosticBag::CreateNoError().Collect(
            Scope::DeclareSymbol(std::make_unique<NormalParamVarSymbol>(
                bodyScope,
                selfName,
                dynamic_cast<ISizedTypeSymbol*>(typeSymbol->GetWithRef()),
                0
            ))
        );

        return glueSymbol;
    }

    static auto TryDeclareAndBindGlueSymbols(
        Compilation* const compilation,
        IConcreteTypeSymbol* const typeSymbol,
        const std::function<FunctionSymbol*(Compilation* const, IConcreteTypeSymbol* const)>& getOrDeclareAndBindGlueSymbols
    ) -> std::optional<FunctionSymbol*>
    {
        if (typeSymbol->IsError())
        {
            return std::nullopt;
        }

        auto* const genericSymbol = dynamic_cast<IGenericSymbol*>(typeSymbol);
        if (genericSymbol && genericSymbol->IsPlaceholder())
        {
            return std::nullopt;
        }

        if (typeSymbol->IsRef())
        {
            return std::nullopt;
        }

        return getOrDeclareAndBindGlueSymbols(compilation, typeSymbol);
    }

    static auto CreateAndBindGlueBlock(
        Compilation* const compilation,
        const std::function<std::shared_ptr<const IEmittable<void>>(IConcreteTypeSymbol* const, FunctionSymbol* const)>& createGlueBlock,
        IConcreteTypeSymbol* const typeSymbol,
        FunctionSymbol* const glueSymbol
    ) -> void
    {
        const auto block = createGlueBlock(typeSymbol, glueSymbol);
        glueSymbol->BindEmittableBlock(block);
    }

    static auto GenerateAndBindGlue(
        Compilation* const compilation,
        const std::vector<IConcreteTypeSymbol*>& typeSymbols,
        const std::function<FunctionSymbol*(Compilation* const, IConcreteTypeSymbol* const)>& getOrDeclareGlueSymbols,
        const std::function<std::shared_ptr<const IEmittable<void>>(IConcreteTypeSymbol* const, FunctionSymbol* const)>& createGlueBlock
    ) -> void
    {
        struct TypeGlueSymbolPair
        {
            IConcreteTypeSymbol* TypeSymbol{};
            FunctionSymbol* FunctionSymbol{};
        };

        std::vector<TypeGlueSymbolPair> typeGlueSymbolPairs{};

        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](IConcreteTypeSymbol* const typeSymbol)
        {
            const auto optGlueSymbol = TryDeclareAndBindGlueSymbols(
                compilation,
                typeSymbol, 
                getOrDeclareGlueSymbols
            );
            if (!optGlueSymbol.has_value())
            {
                return;
            }

            typeGlueSymbolPairs.emplace_back(
                typeSymbol,
                optGlueSymbol.value()
            );
        });

        std::for_each(begin(typeGlueSymbolPairs), end(typeGlueSymbolPairs),
        [&](const TypeGlueSymbolPair& typeGlueSymbolPair)
        {
            CreateAndBindGlueBlock(
                compilation,
                createGlueBlock,
                typeGlueSymbolPair.TypeSymbol,
                typeGlueSymbolPair.FunctionSymbol
            );
        });
    }

    static auto CollectTypeSymbols(
        Compilation* const compilation
    ) -> std::vector<IConcreteTypeSymbol*>
    {
        const auto allTypeSymbols =
            compilation->GetGlobalScope()->CollectSymbolsRecursive<IConcreteTypeSymbol>();
            
        std::vector<IConcreteTypeSymbol*> typeSymbols{};
        std::copy_if(
            begin(allTypeSymbols),
            end  (allTypeSymbols),
            back_inserter(typeSymbols),
            [](IConcreteTypeSymbol* const typeSymbol)
            {
                return !typeSymbol->IsPlaceholder();
            }
        );

        std::set<IConcreteTypeSymbol*> typeSymbolSet{};
        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](IConcreteTypeSymbol* const typeSymbol)
        {
            auto* const unaliasedTypeSymbol =
                dynamic_cast<IConcreteTypeSymbol*>(typeSymbol->GetUnaliased());
            if (unaliasedTypeSymbol)
            {
                typeSymbolSet.insert(unaliasedTypeSymbol);
            }
        });

        return std::vector<IConcreteTypeSymbol*>
        {
            begin(typeSymbolSet),
            end  (typeSymbolSet),
        };
    }

    auto GenerateAndBindGlue(Compilation* const compilation) -> void
    {
        const auto typeSymbols = CollectTypeSymbols(compilation);

        GenerateAndBindGlue(
            compilation,
            typeSymbols,
            &GetOrDeclareAndBindCopyGlueSymbols,
            [](
                IConcreteTypeSymbol* const typeSymbol,
                FunctionSymbol* const glueSymbol
            ) 
            { 
                return typeSymbol->CreateCopyGlueBlock(glueSymbol);
            }
        );
        GenerateAndBindGlue(
            compilation,
            typeSymbols,
            &GetOrDeclareAndBindDropGlueSymbols,
            [](
                IConcreteTypeSymbol* const typeSymbol,
                FunctionSymbol* const glueSymbol
            ) 
            { 
                return typeSymbol->CreateDropGlueBlock(glueSymbol);
            }
        );
    }

    static auto CreateTrivialCopyGlueBlock(
        Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialCopyGlueBlockEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialCopyGlueBlockEmitter(
                IConcreteTypeSymbol* const typeSymbol
            ) : m_TypeSymbol{ typeSymbol }
            {
            }
            ~TrivialCopyGlueBlockEmitter() = default;

            auto Emit(Emitter& emitter) const -> void final
            {
                auto* const    type = emitter.GetType(m_TypeSymbol);
                auto* const ptrType = llvm::PointerType::get(type, 0);

                auto* const  selfPtr = emitter.EmitLoadArg(0, ptrType);
                auto* const otherPtr = emitter.EmitLoadArg(1, ptrType);

                auto* const otherValue = emitter.GetBlock().Builder.CreateLoad(
                    type,
                    otherPtr
                );

                emitter.GetBlock().Builder.CreateStore(otherValue, selfPtr);
                emitter.GetBlock().Builder.CreateRetVoid();
            }

        private:
            IConcreteTypeSymbol* m_TypeSymbol{};
        };

        return std::make_shared<const TrivialCopyGlueBlockEmitter>(
            structSymbol
        );
    }

    static auto CreateTrivialDropGlueBlock(
        Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        IConcreteTypeSymbol* const typeSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialDropGlueBlockEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialDropGlueBlockEmitter() = default;
            ~TrivialDropGlueBlockEmitter() = default;

            auto Emit(Emitter& emitter) const -> void final
            {
                emitter.GetBlock().Builder.CreateRetVoid();
            }
        };

        return std::make_shared<const TrivialDropGlueBlockEmitter>();
    }

    static auto GetCopyOpSymbol(
        ITypeSymbol* const structSymbol
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        auto* const compilation = structSymbol->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetCopyOpMap();
        const auto opSymbolIt = opMap.find(
            dynamic_cast<ITypeSymbol*>(structSymbol->GetRoot())
        );
        if (opSymbolIt == end(opMap))
        {
            return std::nullopt;
        }

        auto* const opSymbol = opSymbolIt->second;

        if (!opSymbol->IsPlaceholder())
        {
            return opSymbol;
        }

        const SrcLocation srcLocation{ compilation };
        const auto& typeArgs = structSymbol->GetTypeArgs();

        return dynamic_cast<FunctionSymbol*>(
            Scope::ForceCollectGenericInstance(opSymbol, typeArgs)
        );
    }

    static auto GetDropOpSymbol(
        ITypeSymbol* const structSymbol
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        auto* const compilation = structSymbol->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetDropOpMap();
        const auto opSymbolIt = opMap.find(
            dynamic_cast<ITypeSymbol*>(structSymbol->GetRoot())
        );
        if (opSymbolIt == end(opMap))
        {
            return std::nullopt;
        }

        auto* const opSymbol = opSymbolIt->second;

        if (!opSymbol->IsPlaceholder())
        {
            return opSymbol;
        }

        const SrcLocation srcLocation{ compilation };
        const auto& typeArgs = structSymbol->GetTypeArgs();

        return dynamic_cast<FunctionSymbol*>(
            Scope::ForceCollectGenericInstance(opSymbol, typeArgs)
        );
    }

    auto CreateCopyGlueBlock(
        Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        if (structSymbol->IsTriviallyCopyable())
        {
            return CreateTrivialCopyGlueBlock(
                compilation,
                glueSymbol,
                structSymbol
            );
        }

        const SrcLocation srcLocation{ compilation };

        const auto blockScope = glueSymbol->GetBodyScope()->CreateChild();

        const auto paramSymbols = glueSymbol->CollectParams();
        const auto selfParamRefExprSema = std::make_shared<const StaticVarRefExprSema>(
            srcLocation,
            blockScope,
            paramSymbols.at(0)
        );
        const auto otherParamRefExprSema = std::make_shared<const StaticVarRefExprSema>(
            srcLocation,
            blockScope,
            paramSymbols.at(1)
        );

        const auto optOpSymbol = GetCopyOpSymbol(structSymbol);

        std::vector<std::shared_ptr<const IStmtSema>> stmts{};
        if (optOpSymbol.has_value())
        {
            std::vector<std::shared_ptr<const IExprSema>> args{};
            args.push_back(selfParamRefExprSema);
            args.push_back(otherParamRefExprSema);

            const auto callExprSema = std::make_shared<const StaticCallExprSema>(
                srcLocation,
                blockScope,
                optOpSymbol.value(),
                args
            );

            const auto exprStmtSema = std::make_shared<const ExprStmtSema>(
                srcLocation,
                callExprSema
            );

            stmts.push_back(exprStmtSema);
        }
        else
        {
            const auto fieldSymbols = structSymbol->CollectFields();
            std::for_each(begin(fieldSymbols), end(fieldSymbols),
            [&](FieldVarSymbol* const fieldSymbol)
            {
                auto* const typeSymbol = dynamic_cast<IConcreteTypeSymbol*>(
                    fieldSymbol->GetSizedType()->GetUnaliased()
                );
                ACE_ASSERT(typeSymbol);

                auto* const typeGlueSymbol = typeSymbol->GetCopyGlue().value();
                
                const auto selfParamVarRefExprSema = std::make_shared<const FieldVarRefExprSema>(
                    srcLocation,
                    selfParamRefExprSema,
                    fieldSymbol
                );
                const auto otherParamVarRefExprSema = std::make_shared<const FieldVarRefExprSema>(
                    srcLocation,
                    otherParamRefExprSema,
                    fieldSymbol
                );

                std::vector<std::shared_ptr<const IExprSema>> args{};
                args.push_back(selfParamVarRefExprSema);
                args.push_back(otherParamVarRefExprSema);

                const auto callExprSema = std::make_shared<const StaticCallExprSema>(
                    srcLocation,
                    blockScope,
                    typeGlueSymbol,
                    args
                );

                const auto exprStmtSema = std::make_shared<const ExprStmtSema>(
                    srcLocation,
                    callExprSema
                );

                stmts.push_back(exprStmtSema);
            });
        }

        const auto blockSema = std::make_shared<const BlockStmtSema>(
            srcLocation,
            blockScope->GetParent().value(),
            stmts
        );

        return diagnostics.Collect(Application::CreateVerifiedFunctionBlock(
            blockSema,
            compilation->GetVoidTypeSymbol()
        ));
    }

    auto CreateDropGlueBlock(
        Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        if (structSymbol->IsTriviallyDroppable())
        {
            return CreateTrivialDropGlueBlock(
                compilation,
                glueSymbol,
                structSymbol
            );
        }

        const SrcLocation srcLocation{ compilation };

        const auto blockScope = glueSymbol->GetBodyScope()->CreateChild();

        const auto paramSymbols = glueSymbol->CollectParams();
        const auto selfParamRefExprSema = std::make_shared<const StaticVarRefExprSema>(
            srcLocation,
            blockScope,
            paramSymbols.at(0)
        );

        const auto optOpSymbol = GetDropOpSymbol(structSymbol);

        std::vector<std::shared_ptr<const IStmtSema>> stmts{};
        if (optOpSymbol.has_value())
        {
            std::vector<std::shared_ptr<const IExprSema>> args{};
            args.push_back(selfParamRefExprSema);

            const auto callExprSema = std::make_shared<const StaticCallExprSema>(
                srcLocation,
                blockScope,
                optOpSymbol.value(),
                args
            );

            const auto exprStmtSema = std::make_shared<const ExprStmtSema>(
                srcLocation,
                callExprSema
            );

            stmts.push_back(exprStmtSema);
        }

        const auto fieldSymbols = structSymbol->CollectFields();
        std::for_each(rbegin(fieldSymbols), rend(fieldSymbols),
        [&](FieldVarSymbol* const fieldSymbol)
        {
            auto* const typeSymbol = dynamic_cast<IConcreteTypeSymbol*>(
                fieldSymbol->GetSizedType()->GetUnaliased()
            );
            ACE_ASSERT(typeSymbol);

            auto* const typeGlueSymbol = typeSymbol->GetDropGlue().value();
            
            const auto varRefExprSema = std::make_shared<const FieldVarRefExprSema>(
                srcLocation,
                selfParamRefExprSema,
                fieldSymbol
            );

            std::vector<std::shared_ptr<const IExprSema>> args{};
            args.push_back(varRefExprSema);

            const auto callExprSema = std::make_shared<const StaticCallExprSema>(
                srcLocation,
                blockScope,
                typeGlueSymbol,
                args
            );

            const auto exprStmtSema = std::make_shared<const ExprStmtSema>(
                srcLocation,
                callExprSema
            );

            stmts.push_back(exprStmtSema);
        });

        auto blockSema = std::make_shared<const BlockStmtSema>(
            srcLocation,
            blockScope->GetParent().value(),
            stmts
        );
        return diagnostics.Collect(Application::CreateVerifiedFunctionBlock(
            blockSema,
            compilation->GetVoidTypeSymbol()
        ));
    }
}
