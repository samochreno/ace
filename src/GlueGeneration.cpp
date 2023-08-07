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
        Compilation* const compilation,
        ITypeSymbol* const typeSymbol
    ) -> FunctionSymbol*
    {
        if (typeSymbol->GetCopyGlue().has_value())
        {
            return typeSymbol->GetCopyGlue().value();
        }

        const auto scope     = typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto nameString = SpecialIdent::CreateCopyGlue(
            typeSymbol->CreatePartialSignature()
        );
        const Ident name
        {
            typeSymbol->GetName().SrcLocation,
            nameString,
        };

        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            compilation->GetNatives().Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();
        typeSymbol->BindCopyGlue(glueSymbol);

        const Ident selfName
        {
            typeSymbol->GetName().SrcLocation,
            SpecialIdent::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            selfName,
            typeSymbol->GetWithRef(),
            0
        )).Unwrap();

        const Ident otherName
        {
            typeSymbol->GetName().SrcLocation,
            SpecialIdent::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            otherName,
            typeSymbol->GetWithRef(),
            1
        )).Unwrap();

        return glueSymbol;
    }

    static auto GetOrDefineAndBindDropGlueSymbols(
        Compilation* const compilation,
        ITypeSymbol* const typeSymbol
    ) -> FunctionSymbol*
    {
        if (typeSymbol->GetDropGlue().has_value())
        {
            return typeSymbol->GetDropGlue().value();
        }

        const auto scope     = typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto nameString = SpecialIdent::CreateDropGlue(
            typeSymbol->CreatePartialSignature()
        );
        const Ident name
        {
            typeSymbol->GetName().SrcLocation,
            nameString,
        };

        auto ownedGlueSymbol = std::make_unique<FunctionSymbol>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            compilation->GetNatives().Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();
        typeSymbol->BindDropGlue(glueSymbol);

        const Ident selfName
        {
            typeSymbol->GetName().SrcLocation,
            SpecialIdent::CreateAnonymous(),
        };
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            selfName,
            typeSymbol->GetWithRef(),
            0
        )).Unwrap();

        return glueSymbol;
    }

    static auto TryDefineAndBindGlueSymbols(
        Compilation* const compilation,
        ITypeSymbol* const typeSymbol,
        const std::function<FunctionSymbol*(Compilation* const, ITypeSymbol* const)>& getOrDefineAndBindGlueSymbols
    ) -> std::optional<FunctionSymbol*>
    {
        if (typeSymbol->IsError())
        {
            return std::nullopt;
        }

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

        if (typeSymbol->IsRef())
        {
            return std::nullopt;
        }

        return getOrDefineAndBindGlueSymbols(compilation, typeSymbol);
    }

    static auto CreateAndBindGlueBody(
        Compilation* const compilation,
        const std::function<std::shared_ptr<const IEmittable<void>>(ITypeSymbol* const, FunctionSymbol* const)>& createGlueBody,
        ITypeSymbol* const typeSymbol,
        FunctionSymbol* const glueSymbol
    ) -> void
    {
        const auto body = createGlueBody(typeSymbol, glueSymbol);
        glueSymbol->BindBody(body);
    }

    static auto GenerateAndBindGlue(
        Compilation* const compilation,
        const std::function<FunctionSymbol*(Compilation* const, ITypeSymbol* const)>& getOrDefineGlueSymbols,
        const std::function<std::shared_ptr<const IEmittable<void>>(ITypeSymbol* const, FunctionSymbol* const)>& createGlueBody
    ) -> void
    {
        const auto typeSymbols =
            compilation->GetGlobalScope()->CollectSymbolsRecursive<ITypeSymbol>();

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

            typeGlueSymbolPairs.emplace_back(
                typeSymbol,
                optGlueSymbol.value()
            );
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
        Compilation* const compilation
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
        Compilation* const compilation,
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
        Compilation* const compilation,
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
        Compilation* const compilation,
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
        const auto selfParamRefExprNode = std::make_shared<const StaticVarRefExprBoundNode>(
            SrcLocation{},
            bodyScope,
            paramSymbols.at(0)
        );
        const auto otherParamRefExprNode = std::make_shared<const StaticVarRefExprBoundNode>(
            SrcLocation{},
            bodyScope,
            paramSymbols.at(1)
        );

        auto opName = structSymbol->CreateFullyQualifiedName(
            structSymbol->GetName().SrcLocation
        );
        opName.Sections.emplace_back(Ident{
            structSymbol->GetName().SrcLocation,
            SpecialIdent::Op::Copy,
        });
        const auto expOpSymbol = 
            compilation->GetGlobalScope()->ResolveStaticSymbol<FunctionSymbol>(opName);

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};
        if (expOpSymbol)
        {
            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamRefExprNode);
            args.push_back(otherParamRefExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SrcLocation{},
                bodyScope,
                expOpSymbol.Unwrap(),
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                SrcLocation{},
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        }
        else
        {
            const auto varSymbols = structSymbol->CollectVars();
            std::for_each(begin(varSymbols), end(varSymbols),
            [&](InstanceVarSymbol* const varSymbol)
            {
                auto* const varTypeSymbol = varSymbol->GetType();
                auto* const varTypeGlueSymbol =
                    varTypeSymbol->GetCopyGlue().value();
                
                const auto selfParamVarRerefenceExprNode = std::make_shared<const InstanceVarRefExprBoundNode>(
                    SrcLocation{},
                    selfParamRefExprNode,
                    varSymbol
                );
                const auto otherParamVarRerefenceExprNode = std::make_shared<const InstanceVarRefExprBoundNode>(
                    SrcLocation{},
                    otherParamRefExprNode,
                    varSymbol
                );

                std::vector<std::shared_ptr<const IExprBoundNode>> args{};
                args.push_back(selfParamVarRerefenceExprNode);
                args.push_back(otherParamVarRerefenceExprNode);

                const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                    SrcLocation{},
                    bodyScope,
                    varTypeGlueSymbol,
                    args
                );

                const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                    SrcLocation{},
                    functionCallExprNode
                );

                stmts.push_back(exprStmtNode);
            });
        }

        const auto bodyNode = std::make_shared<const BlockStmtBoundNode>(
            SrcLocation{},
            bodyScope->GetParent().value(),
            stmts
        );

        return Application::CreateTransformedAndVerifiedAST(
            bodyNode,
            [&](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            { 
                return bodyNode->CreateTypeChecked({
                    compilation->GetNatives().Void.GetSymbol()
                }); 
            },
            [](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            {
                return bodyNode->CreateLowered({});
            }
        ).Unwrap();
    }

    auto CreateDropGlueBody(
        Compilation* const compilation,
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
        const auto selfParamRefExprNode = std::make_shared<const StaticVarRefExprBoundNode>(
            SrcLocation{},
            bodyScope,
            paramSymbols.at(0)
        );

        std::vector<std::shared_ptr<const IStmtBoundNode>> stmts{};

        auto opName = structSymbol->CreateFullyQualifiedName(
            structSymbol->GetName().SrcLocation
        );
        opName.Sections.emplace_back(Ident{
            structSymbol->GetName().SrcLocation,
            SpecialIdent::Op::Drop,
        });
        const auto expOpSymbol = 
            compilation->GetGlobalScope()->ResolveStaticSymbol<FunctionSymbol>(opName);

        if (expOpSymbol)
        {
            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamRefExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SrcLocation{},
                bodyScope,
                expOpSymbol.Unwrap(),
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                SrcLocation{},
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        }

        const auto varSymbols = structSymbol->CollectVars();
        std::for_each(rbegin(varSymbols), rend(varSymbols),
        [&](InstanceVarSymbol* const varSymbol)
        {
            auto* const varTypeSymbol = varSymbol->GetType();
            auto* const varTypeGlueSymbol = 
                varTypeSymbol->GetDropGlue().value();
            
            const auto selfParamVarRerefenceExprNode = std::make_shared<const InstanceVarRefExprBoundNode>(
                SrcLocation{},
                selfParamRefExprNode,
                varSymbol
            );

            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamVarRerefenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                SrcLocation{},
                bodyScope,
                varTypeGlueSymbol,
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                SrcLocation{},
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        });

        const auto bodyNode = std::make_shared<const BlockStmtBoundNode>(
            SrcLocation{},
            bodyScope->GetParent().value(),
            stmts
        );

        return Application::CreateTransformedAndVerifiedAST(
            bodyNode,
            [&](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            { 
                return bodyNode->CreateTypeChecked({
                    compilation->GetNatives().Void.GetSymbol()
                }); 
            },
            [](const std::shared_ptr<const BlockStmtBoundNode>& bodyNode)
            {
                return bodyNode->CreateLowered({});
            }
        ).Unwrap();
    }
}
