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
    static auto GetOrDefineCopyGlueSymbols(
        const Compilation* const t_compilation,
        ITypeSymbol* const t_typeSymbol
    ) -> FunctionSymbol*
    {
        const auto scope     = t_typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto nameString = SpecialIdentifier::CreateCopyGlue(
            t_typeSymbol->CreatePartialSignature()
        );

        if (const auto expGlueSymbol = scope->ExclusiveResolveSymbol<FunctionSymbol>(nameString))
        {
            return expGlueSymbol.Unwrap();
        }

        const Identifier name
        {
            t_typeSymbol->GetName().SourceLocation,
            nameString,
        };
        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            t_compilation->Natives->Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();

        const Identifier selfName
        {
            t_typeSymbol->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            selfName,
            t_typeSymbol->GetWithReference(),
            0
        )).Unwrap();

        const Identifier otherName
        {
            t_typeSymbol->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            otherName,
            t_typeSymbol->GetWithReference(),
            1
        )).Unwrap();

        return glueSymbol;
    }

    static auto GetOrDefineDropGlueSymbols(
        const Compilation* const t_compilation,
        ITypeSymbol* const t_typeSymbol
    ) -> FunctionSymbol*
    {
        const auto scope     = t_typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto nameString = SpecialIdentifier::CreateDropGlue(
            t_typeSymbol->CreatePartialSignature()
        );

        if (const auto expGlueSymbol = scope->ExclusiveResolveSymbol<FunctionSymbol>(nameString))
        {
            return expGlueSymbol.Unwrap();
        }

        const Identifier name
        {
            t_typeSymbol->GetName().SourceLocation,
            nameString,
        };
        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            t_compilation->Natives->Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();

        const Identifier selfName
        {
            t_typeSymbol->GetName().SourceLocation,
            SpecialIdentifier::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            selfName,
            t_typeSymbol->GetWithReference(),
            0
        )).Unwrap();

        return glueSymbol;
    }

    static auto TryDefineGlueSymbols(
        const Compilation* const t_compilation,
        ITypeSymbol* const t_typeSymbol,
        const std::function<FunctionSymbol*(const Compilation* const, ITypeSymbol* const)>& t_getOrDefineGlueSymbols
    ) -> std::optional<FunctionSymbol*>
    {
        auto* const templatableSymbol = dynamic_cast<ITemplatableSymbol*>(
            t_typeSymbol
        );
        if (templatableSymbol && templatableSymbol->IsTemplatePlaceholder())
        {
            return std::nullopt;
        }

        if (t_typeSymbol->GetSizeKind().Unwrap() == TypeSizeKind::Unsized)
        {
            return std::nullopt;
        }

        if (t_typeSymbol->IsReference())
        {
            return std::nullopt;
        }

        return t_getOrDefineGlueSymbols(t_compilation, t_typeSymbol);
    }

    static auto CreateAndBindGlueBody(
        const Compilation* const t_compilation,
        const std::function<std::shared_ptr<const IEmittable<void>>(ITypeSymbol* const, FunctionSymbol* const)>& t_createGlueBody,
        ITypeSymbol* const t_typeSymbol,
        FunctionSymbol* const t_glueSymbol
    ) -> void
    {
        const auto body = t_createGlueBody(t_typeSymbol, t_glueSymbol);
        t_glueSymbol->BindBody(body);
    }

    static auto GenerateAndBindGlue(
        const Compilation* const t_compilation,
        const std::function<FunctionSymbol*(const Compilation* const, ITypeSymbol* const)>& t_getOrDefineGlueSymbols,
        const std::function<std::shared_ptr<const IEmittable<void>>(ITypeSymbol* const, FunctionSymbol* const)>& t_createGlueBody,
        const std::function<void(ITypeSymbol* const, FunctionSymbol* const)>& t_bindGlue
    ) -> void
    {
        const auto typeSymbols =
            t_compilation->GlobalScope.Unwrap()->CollectSymbolsRecursive<ITypeSymbol>();

        struct TypeGlueSymbolPair
        {
            ITypeSymbol* TypeSymbol{};
            FunctionSymbol* FunctionSymbol{};
        };

        std::vector<TypeGlueSymbolPair> typeGlueSymbolPairs{};

        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](ITypeSymbol* const t_typeSymbol)
        {
            const auto optGlueSymbol = TryDefineGlueSymbols(
                t_compilation,
                t_typeSymbol, 
                t_getOrDefineGlueSymbols
            );
            if (!optGlueSymbol.has_value())
            {
                return;
            }

            typeGlueSymbolPairs.push_back(TypeGlueSymbolPair{ 
                t_typeSymbol,
                optGlueSymbol.value()
            });
        });

        std::for_each(begin(typeGlueSymbolPairs), end(typeGlueSymbolPairs),
        [&](const TypeGlueSymbolPair& t_typeGlueSymbolPair)
        {
            CreateAndBindGlueBody(
                t_compilation,
                t_createGlueBody,
                t_typeGlueSymbolPair.TypeSymbol,
                t_typeGlueSymbolPair.FunctionSymbol
            );

            t_bindGlue(
                t_typeGlueSymbolPair.TypeSymbol,
                t_typeGlueSymbolPair.FunctionSymbol
            );
        });
    }

    auto GenerateAndBindGlue(
        const Compilation* const t_compilation
    ) -> void
    {
        GenerateAndBindGlue(
            t_compilation,
            &GetOrDefineCopyGlueSymbols,
            [](
                ITypeSymbol* const t_typeSymbol, 
                FunctionSymbol* const t_glueSymbol
            ) 
            { 
                return t_typeSymbol->CreateCopyGlueBody(t_glueSymbol);
            },
            [](
                ITypeSymbol* const t_typeSymbol, 
                FunctionSymbol* const t_glueSymbol
            ) 
            { 
                t_typeSymbol->BindCopyGlue(t_glueSymbol);
            }
        );
        GenerateAndBindGlue(
            t_compilation,
            &GetOrDefineDropGlueSymbols,
            [](
                ITypeSymbol* const t_typeSymbol, 
                FunctionSymbol* const t_glueSymbol
            ) 
            { 
                return t_typeSymbol->CreateDropGlueBody(t_glueSymbol);
            },
            [](
                ITypeSymbol* const t_typeSymbol, 
                FunctionSymbol* const t_glueSymbol
            ) 
            { 
                t_typeSymbol->BindDropGlue(t_glueSymbol);
            }
        );
    }

    static auto CreateTrivialCopyGlueBody(
        const Compilation* const t_compilation,
        FunctionSymbol* const t_glueSymbol,
        StructTypeSymbol* const t_structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialCopyGlueBodyEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialCopyGlueBodyEmitter(
                ITypeSymbol* const t_typeSymbol
            ) : m_TypeSymbol{ t_typeSymbol }
            {
            }
            ~TrivialCopyGlueBodyEmitter() = default;

            auto Emit(Emitter& t_emitter) const -> void final
            {
                auto* const type = t_emitter.GetIRType(m_TypeSymbol);
                auto* const ptrType = llvm::PointerType::get(
                    type,
                    0
                );
                
                auto* const selfPtr = t_emitter.EmitLoadArg(
                    0,
                    ptrType
                );

                auto* const otherPtr = t_emitter.EmitLoadArg(
                    1, 
                    ptrType
                );
                auto* const otherValue = t_emitter.GetBlockBuilder().Builder.CreateLoad(
                    type,
                    otherPtr
                );

                t_emitter.GetBlockBuilder().Builder.CreateStore(
                    otherValue,
                    selfPtr
                );

                t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }

        private:
            ITypeSymbol* m_TypeSymbol{};
        };

        return std::make_shared<const TrivialCopyGlueBodyEmitter>(
            t_structSymbol
        );
    }

    static auto CreateTrivialDropGlueBody(
        const Compilation* const t_compilation,
        FunctionSymbol* const t_glueSymbol,
        ITypeSymbol* const t_typeSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialDropGlueBodyEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialDropGlueBodyEmitter() = default;
            ~TrivialDropGlueBodyEmitter() = default;

            auto Emit(Emitter& t_emitter) const -> void final
            {
                t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
            }
        };

        return std::make_shared<const TrivialDropGlueBodyEmitter>();
    }

    auto CreateCopyGlueBody(
        const Compilation* const t_compilation,
        FunctionSymbol* const t_glueSymbol,
        StructTypeSymbol* const t_structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        if (t_structSymbol->IsTriviallyCopyable())
        {
            return CreateTrivialCopyGlueBody(
                t_compilation,
                t_glueSymbol,
                t_structSymbol
            );
        }

        const auto bodyScope = t_glueSymbol->GetSelfScope()->GetOrCreateChild({});

        const auto paramSymbols = t_glueSymbol->CollectParams();
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

        auto operatorName = t_structSymbol->CreateFullyQualifiedName();
        operatorName.Sections.emplace_back(SpecialIdentifier::Operator::Copy);
        const auto expOperatorSymbol = 
            t_compilation->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionSymbol>(operatorName);

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};
        if (expOperatorSymbol)
        {
            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamReferenceExprNode);
            args.push_back(otherParamReferenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SourceLocation{},
                bodyScope,
                expOperatorSymbol.Unwrap(),
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
            const auto variableSymbols = t_structSymbol->GetVars();
            std::for_each(begin(variableSymbols), end(variableSymbols),
            [&](InstanceVarSymbol* const t_variableSymbol)
            {
                auto* const variableTypeSymbol = t_variableSymbol->GetType();
                const auto variableTypeScope = variableTypeSymbol->GetUnaliased()->GetScope();
                auto* const variableTypeGlueSymbol = variableTypeScope->ExclusiveResolveSymbol<FunctionSymbol>(
                    SpecialIdentifier::CreateCopyGlue(variableTypeSymbol->CreatePartialSignature())
                ).Unwrap();
                
                const auto selfParamVarRerefenceExprNode = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                    SourceLocation{},
                    selfParamReferenceExprNode,
                    t_variableSymbol
                );
                const auto otherParamVarRerefenceExprNode = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                    SourceLocation{},
                    otherParamReferenceExprNode,
                    t_variableSymbol
                );

                std::vector<std::shared_ptr<const IExprBoundNode>> args{};
                args.push_back(selfParamVarRerefenceExprNode);
                args.push_back(otherParamVarRerefenceExprNode);

                const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                    SourceLocation{},
                    bodyScope,
                    variableTypeGlueSymbol,
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
            t_compilation,
            bodyNode,
            [&](const std::shared_ptr<const BlockStmtBoundNode>& t_bodyNode)
            { 
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation->Natives->Void.GetSymbol() }
                ); 
            },
            [](const std::shared_ptr<const BlockStmtBoundNode>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateLowered({});
            }
        ).Unwrap();
    }

    auto CreateDropGlueBody(
        const Compilation* const t_compilation,
        FunctionSymbol* const t_glueSymbol,
        StructTypeSymbol* const t_structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        if (t_structSymbol->IsTriviallyDroppable())
        {
            return CreateTrivialDropGlueBody(
                t_compilation,
                t_glueSymbol,
                t_structSymbol
            );
        }

        const auto bodyScope = t_glueSymbol->GetSelfScope()->GetOrCreateChild({});

        const auto paramSymbols = t_glueSymbol->CollectParams();
        const auto selfParamReferenceExprNode = std::make_shared<const StaticVarReferenceExprBoundNode>(
            SourceLocation{},
            bodyScope,
            paramSymbols.at(0)
        );

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        auto operatorName = t_structSymbol->CreateFullyQualifiedName();
        operatorName.Sections.emplace_back(SpecialIdentifier::Operator::Drop);
        const auto expOperatorSymbol = 
            t_compilation->GlobalScope.Unwrap()->ResolveStaticSymbol<FunctionSymbol>(operatorName);

        if (expOperatorSymbol)
        {
            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamReferenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SourceLocation{},
                bodyScope,
                expOperatorSymbol.Unwrap(),
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                SourceLocation{},
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        }

        const auto variableSymbols = t_structSymbol->GetVars();
        std::for_each(rbegin(variableSymbols), rend(variableSymbols),
        [&](InstanceVarSymbol* const t_variableSymbol)
        {
            auto* const variableTypeSymbol = t_variableSymbol->GetType();
            const auto variableTypeScope = variableTypeSymbol->GetUnaliased()->GetScope();
            auto* const variableTypeGlueSymbol = variableTypeScope->ExclusiveResolveSymbol<FunctionSymbol>(
                SpecialIdentifier::CreateDropGlue(variableTypeSymbol->CreatePartialSignature())
            ).Unwrap();
            
            const auto selfParamVarRerefenceExprNode = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                SourceLocation{},
                selfParamReferenceExprNode,
                t_variableSymbol
            );

            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamVarRerefenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SourceLocation{},
                bodyScope,
                variableTypeGlueSymbol,
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
            t_compilation,
            bodyNode,
            [&](const std::shared_ptr<const BlockStmtBoundNode>& t_bodyNode)
            { 
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation->Natives->Void.GetSymbol() }
                ); 
            },
            [](const std::shared_ptr<const BlockStmtBoundNode>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateLowered({});
            }
        ).Unwrap();
    }
}
