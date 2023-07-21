#include "GlueGeneration.hpp"

#include <memory>
#include <vector>

#include "Application.hpp"
#include "Compilation.hpp"
#include "Emittable.hpp"
#include "Emitter.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"

namespace Ace::GlueGeneration
{
    static auto GetOrDefineAndBindCopyGlueSymbols(
        const Compilation* const compilation,
        ITypeSymbol* const typeSymbol
    ) -> FunctionSymbol*
    {
        if (typeSymbol->GetCopyGlue().has_value())
        {
            return typeSymbol->GetCopyGlue().value();
        }

        const auto scope     = typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto nameString = SpecialIdentifier::CreateCopyGlue(
            typeSymbol->CreatePartialSignature()
        );
        const Identifier name
        {
            typeSymbol->GetName().SourceLocation,
            nameString,
        };

        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            compilation->Natives->Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();
        typeSymbol->BindCopyGlue(glueSymbol);

        const Identifier selfName
        {
            typeSymbol->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            selfName,
            typeSymbol->GetWithReference(),
            0
        )).Unwrap();

        const Identifier otherName
        {
            typeSymbol->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            otherName,
            typeSymbol->GetWithReference(),
            1
        )).Unwrap();

        return glueSymbol;
    }

    static auto GetOrDefineAndBindDropGlueSymbols(
        const Compilation* const compilation,
        ITypeSymbol* const typeSymbol
    ) -> FunctionSymbol*
    {
        if (typeSymbol->GetDropGlue().has_value())
        {
            return typeSymbol->GetDropGlue().value();
        }

        const auto scope     = typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto nameString = SpecialIdentifier::CreateDropGlue(
            typeSymbol->CreatePartialSignature()
        );
        const Identifier name
        {
            typeSymbol->GetName().SourceLocation,
            nameString,
        };

        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            compilation->Natives->Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();
        typeSymbol->BindDropGlue(glueSymbol);

        const Identifier selfName
        {
            typeSymbol->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            selfName,
            typeSymbol->GetWithReference(),
            0
        )).Unwrap();

        return glueSymbol;
    }

    static auto TryDefineAndBindGlueSymbols(
        const Compilation* const compilation,
        ITypeSymbol* const typeSymbol,
        const std::function<FunctionSymbol*(const Compilation* const, ITypeSymbol* const)>& getOrDefineAndBindGlueSymbols
    ) -> std::optional<FunctionSymbol*>
    {
        auto* const templatableSymbol = dynamic_cast<ITemplatableSymbol*>(
            typeSymbol
        );
        if (templatableSymbol && templatableSymbol->IsTemplatePlaceholder())
        {
            return std::nullopt;
        }

        if (typeSymbol->GetSizeKind().Unwrap() == TypeSizeKind::Unsized)
        {
            return std::nullopt;
        }

        if (typeSymbol->IsReference())
        {
            return std::nullopt;
        }

        return getOrDefineAndBindGlueSymbols(compilation, typeSymbol);
    }

    static auto CreateAndBindGlueBody(
        const Compilation* const compilation,
        const std::function<std::shared_ptr<const IEmittable<void>>(ITypeSymbol* const, FunctionSymbol* const)>& createGlueBody,
        ITypeSymbol* const typeSymbol,
        FunctionSymbol* const glueSymbol
    ) -> void
    {
        const auto body = createGlueBody(typeSymbol, glueSymbol);
        glueSymbol->BindBody(body);
    }

    static auto GenerateAndBindGlue(
        const Compilation* const compilation,
        const std::function<FunctionSymbol*(const Compilation* const, ITypeSymbol* const)>& getOrDefineGlueSymbols,
        const std::function<std::shared_ptr<const IEmittable<void>>(ITypeSymbol* const, FunctionSymbol* const)>& createGlueBody
    ) -> void
    {
        const auto typeSymbols =
            compilation->GlobalScope.Unwrap()->CollectSymbolsRecursive<ITypeSymbol>();

        struct TypeGlueSymbolPair
        {
            ITypeSymbol* TypeSymbol{};
            FunctionSymbol* FunctionSymbol{};
        };

        std::vector<TypeGlueSymbolPair> typeGlueSymbolPairs{};

        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](ITypeSymbol* const typeSymbol)
        {
            const auto optGlueSymbol = TryDefineAndBindGlueSymbols(
                compilation,
                typeSymbol, 
                getOrDefineGlueSymbols
            );
            if (!optGlueSymbol.has_value())
            {
                return;
            }

            typeGlueSymbolPairs.push_back(TypeGlueSymbolPair{ 
                typeSymbol,
                optGlueSymbol.value()
            });
        });

        std::for_each(begin(typeGlueSymbolPairs), end(typeGlueSymbolPairs),
        [&](const TypeGlueSymbolPair& typeGlueSymbolPair)
        {
            CreateAndBindGlueBody(
                compilation,
                createGlueBody,
                typeGlueSymbolPair.TypeSymbol,
                typeGlueSymbolPair.FunctionSymbol
            );
        });
    }

    auto GenerateAndBindGlue(
        const Compilation* const compilation
    ) -> void
    {
        GenerateAndBindGlue(
            compilation,
            &GetOrDefineAndBindCopyGlueSymbols,
            [](
                ITypeSymbol* const typeSymbol, 
                FunctionSymbol* const glueSymbol
            ) 
            { 
                return typeSymbol->CreateCopyGlueBody(glueSymbol);
            }
        );
        GenerateAndBindGlue(
            compilation,
            &GetOrDefineAndBindDropGlueSymbols,
            [](
                ITypeSymbol* const typeSymbol, 
                FunctionSymbol* const glueSymbol
            ) 
            { 
                return typeSymbol->CreateDropGlueBody(glueSymbol);
            }
        );
    }

    static auto CreateTrivialCopyGlueBody(
        const Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialCopyGlueBodyEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialCopyGlueBodyEmitter(
                ITypeSymbol* const typeSymbol
            ) : m_TypeSymbol{ typeSymbol }
            {
            }
            ~TrivialCopyGlueBodyEmitter() = default;

            auto Emit(Emitter& emitter) const -> void final
            {
                auto* const type = emitter.GetIRType(m_TypeSymbol);
                auto* const ptrType = llvm::PointerType::get(
                    type,
                    0
                );
                
                auto* const selfPtr = emitter.EmitLoadArg(
                    0,
                    ptrType
                );

                auto* const otherPtr = emitter.EmitLoadArg(
                    1, 
                    ptrType
                );
                auto* const otherValue = emitter.GetBlockBuilder().Builder.CreateLoad(
                    type,
                    otherPtr
                );

                emitter.GetBlockBuilder().Builder.CreateStore(
                    otherValue,
                    selfPtr
                );

                emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }

        private:
            ITypeSymbol* m_TypeSymbol{};
        };

        return std::make_shared<const TrivialCopyGlueBodyEmitter>(
            structSymbol
        );
    }

    static auto CreateTrivialDropGlueBody(
        const Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        ITypeSymbol* const typeSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialDropGlueBodyEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialDropGlueBodyEmitter() = default;
            ~TrivialDropGlueBodyEmitter() = default;

            auto Emit(Emitter& emitter) const -> void final
            {
                emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        };

        return std::make_shared<const TrivialDropGlueBodyEmitter>();
    }

    auto CreateCopyGlueBody(
        const Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        if (structSymbol->IsTriviallyCopyable())
        {
            return CreateTrivialCopyGlueBody(
                compilation,
                glueSymbol,
                structSymbol
            );
        }

        const auto bodyScope = glueSymbol->GetSelfScope()->GetOrCreateChild({});

        const auto paramSymbols = glueSymbol->CollectParams();
        const auto selfParamReferenceExprNode = std::make_shared<const StaticVarReferenceExprBoundNode>(
            SourceLocation{},
            bodyScope,
            paramSymbols.at(0)
        );
        const auto otherParamReferenceExprNode = std::make_shared<const StaticVarReferenceExprBoundNode>(
            SourceLocation{},
            bodyScope,
            paramSymbols.at(1)
        );

        auto opName = structSymbol->CreateFullyQualifiedName(
            structSymbol->GetName().SourceLocation
        );
        opName.Sections.push_back(Identifier{
            structSymbol->GetName().SourceLocation,
            SpecialIdentifier::Op::Copy,
        });
        const auto expOpSymbol = 
            compilation->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionSymbol>(opName);

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};
        if (expOpSymbol)
        {
            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamReferenceExprNode);
            args.push_back(otherParamReferenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SourceLocation{},
                bodyScope,
                expOpSymbol.Unwrap(),
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                SourceLocation{},
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        }
        else
        {
            const auto varSymbols = structSymbol->GetVars();
            std::for_each(begin(varSymbols), end(varSymbols),
            [&](InstanceVarSymbol* const varSymbol)
            {
                auto* const varTypeSymbol = varSymbol->GetType();
                auto* const varTypeGlueSymbol =
                    varTypeSymbol->GetCopyGlue().value();
                
                const auto selfParamVarRerefenceExprNode = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                    SourceLocation{},
                    selfParamReferenceExprNode,
                    varSymbol
                );
                const auto otherParamVarRerefenceExprNode = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                    SourceLocation{},
                    otherParamReferenceExprNode,
                    varSymbol
                );

                std::vector<std::shared_ptr<const IExprBoundNode>> args{};
                args.push_back(selfParamVarRerefenceExprNode);
                args.push_back(otherParamVarRerefenceExprNode);

                const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                    SourceLocation{},
                    bodyScope,
                    varTypeGlueSymbol,
                    args
                );

                const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                    SourceLocation{},
                    functionCallExprNode
                );

                stmts.push_back(exprStmtNode);
            });
        }

        const auto bodyNode = std::make_shared<const BlockStmtBoundNode>(
            SourceLocation{},
            bodyScope->GetParent().value(),
            stmts
        );

        return Application::CreateTransformedAndVerifiedAST(
            compilation,
            bodyNode,
            [&](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            { 
                return bodyNode->GetOrCreateTypeChecked(
                    { compilation->Natives->Void.GetSymbol() }
                ); 
            },
            [](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            {
                return bodyNode->GetOrCreateLowered({});
            }
        ).Unwrap();
    }

    auto CreateDropGlueBody(
        const Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        if (structSymbol->IsTriviallyDroppable())
        {
            return CreateTrivialDropGlueBody(
                compilation,
                glueSymbol,
                structSymbol
            );
        }

        const auto bodyScope = glueSymbol->GetSelfScope()->GetOrCreateChild({});

        const auto paramSymbols = glueSymbol->CollectParams();
        const auto selfParamReferenceExprNode = std::make_shared<const StaticVarReferenceExprBoundNode>(
            SourceLocation{},
            bodyScope,
            paramSymbols.at(0)
        );

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        auto opName = structSymbol->CreateFullyQualifiedName(
            structSymbol->GetName().SourceLocation
        );
        opName.Sections.push_back(Identifier{
            structSymbol->GetName().SourceLocation,
            SpecialIdentifier::Op::Drop,
        });
        const auto expOpSymbol = 
            compilation->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionSymbol>(opName);

        if (expOpSymbol)
        {
            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamReferenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SourceLocation{},
                bodyScope,
                expOpSymbol.Unwrap(),
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                SourceLocation{},
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        }

        const auto varSymbols = structSymbol->GetVars();
        std::for_each(rbegin(varSymbols), rend(varSymbols),
        [&](InstanceVarSymbol* const varSymbol)
        {
            auto* const varTypeSymbol = varSymbol->GetType();
            auto* const varTypeGlueSymbol = 
                varTypeSymbol->GetDropGlue().value();
            
            const auto selfParamVarRerefenceExprNode = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                SourceLocation{},
                selfParamReferenceExprNode,
                varSymbol
            );

            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamVarRerefenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SourceLocation{},
                bodyScope,
                varTypeGlueSymbol,
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                SourceLocation{},
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        });

        const auto bodyNode = std::make_shared<const BlockStmtBoundNode>(
            SourceLocation{},
            bodyScope->GetParent().value(),
            stmts
        );

        return Application::CreateTransformedAndVerifiedAST(
            compilation,
            bodyNode,
            [&](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            { 
                return bodyNode->GetOrCreateTypeChecked(
                    { compilation->Natives->Void.GetSymbol() }
                ); 
            },
            [](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            {
                return bodyNode->GetOrCreateLowered({});
            }
        ).Unwrap();
    }
}
