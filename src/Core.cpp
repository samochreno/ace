#include "Core.hpp"

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Nodes/Node.hpp"
#include "Nodes/ModuleNode.hpp"
#include "Nodes/ImplNode.hpp"
#include "BoundNodes/BoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Log.hpp"
#include "Utility.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SpecialIdentifier.hpp"
#include "Emitter.hpp"
#include "Emittable.hpp"
#include "Compilation.hpp"
#include "ControlFlowAnalysis.hpp"
#include "FileBuffer.hpp"

namespace Ace::Core
{
    auto LogDiagnosticBag(const DiagnosticBag& t_diagnosticBag) -> void
    {
        std::for_each(
            begin(t_diagnosticBag.GetDiagnostics()),
            end  (t_diagnosticBag.GetDiagnostics()),
            [](const std::shared_ptr<const Diagnostic>& t_diagnostic)
            {
                Log(
                    t_diagnostic->Severity,
                    t_diagnostic->Message,
                    t_diagnostic->OptSourceLocation
                );
            }
        );
    }

    auto ParseAST(
        const Compilation* const t_compilation,
        const FileBuffer* const t_fileBuffer
    ) -> Diagnosed<std::shared_ptr<const ModuleNode>>
    {
        DiagnosticBag diagnosticBag{};

        Lexer lexer{ t_fileBuffer };
        auto dgnTokens = lexer.EatTokens();
        diagnosticBag.Add(dgnTokens.GetDiagnosticBag());

        LogDiagnosticBag(diagnosticBag);

        const auto ast = Parser::ParseAST(
            t_fileBuffer,
            std::move(dgnTokens.Unwrap())
        ).Unwrap();

        return Diagnosed
        {
            ast,
            diagnosticBag,
        };
    }

    auto CreateAndDefineSymbols(
        const Compilation* const t_compilation,
        const std::vector<const INode*>& t_nodes
    ) -> Expected<void>
    {
        auto symbolCreatableNodes = DynamicCastFilter<const ISymbolCreatableNode*>(t_nodes);
        std::sort(begin(symbolCreatableNodes), end(symbolCreatableNodes),
        [](
            const ISymbolCreatableNode* const t_lhs,
            const ISymbolCreatableNode* const t_rhs
        )
        {
            const auto lhsCreationOrder =
                GetSymbolCreationOrder(t_lhs->GetSymbolKind());

            const auto rhsCreationOrder =
                GetSymbolCreationOrder(t_rhs->GetSymbolKind());

            if (lhsCreationOrder < rhsCreationOrder)
            {
                return true;
            }

            if (lhsCreationOrder > rhsCreationOrder)
            {
                return false;
            }

            const auto lhsKindSpecificCreationOrder =
                t_lhs->GetSymbolCreationSuborder();

            const auto rhsKindSpecificCreationOrder =
                t_rhs->GetSymbolCreationSuborder();

            if (lhsKindSpecificCreationOrder < rhsKindSpecificCreationOrder)
            {
                return true;
            }

            if (lhsKindSpecificCreationOrder > rhsKindSpecificCreationOrder)
            {
                return false;
            }

            return false;
        });

        ACE_TRY_VOID(TransformExpectedVector(symbolCreatableNodes,
        [](const ISymbolCreatableNode* const t_symbolCreatableNode) -> Expected<void>
        {
            ACE_TRY(symbol, Scope::DefineSymbol(t_symbolCreatableNode));
            return Void{};
        }));

        return Void{};
    }

    auto DefineAssociations(
        const Compilation* const t_compilation,
        const std::vector<const INode*>& t_nodes
    ) -> Expected<void>
    {
        const auto implNodes = DynamicCastFilter<const ImplNode*>(t_nodes);

        const auto didCreateAssociations = TransformExpectedVector(implNodes,
        [](const ImplNode* const t_implNode) -> Expected<void>
        {
            ACE_TRY_VOID(t_implNode->DefineAssociations());
            return Void{};
        });
        ACE_TRY_ASSERT(didCreateAssociations);

        return Void{};
    }

    auto ValidateControlFlow(
        const Compilation* const t_compilation,
        const std::vector<const IBoundNode*>& t_nodes
    ) -> Expected<void>
    {
        const auto functionNodes =
            DynamicCastFilter<const FunctionBoundNode*>(t_nodes);

        const bool didControlFlowAnalysisSucceed = std::find_if(
            begin(functionNodes), 
            end  (functionNodes),
            [&](const FunctionBoundNode* const t_functionNode)
            {
                if (
                    t_functionNode->GetSymbol()->GetType()->GetUnaliased() == 
                    t_compilation->Natives->Void.GetSymbol()
                    )
                {
                    return false;
                }

                if (!t_functionNode->GetBody().has_value())
                {
                    return false;
                }

                ControlFlowAnalysis controlFlowAnalysis
                {
                    t_functionNode->GetBody().value()
                };

                return controlFlowAnalysis.IsEndReachableWithoutReturn();
            }
        ) == end(functionNodes);
        ACE_TRY_ASSERT(didControlFlowAnalysisSucceed);

        return Void{};
    }

    auto BindFunctionSymbolsBodies(
        const Compilation* const t_compilation,
        const std::vector<const IBoundNode*>& t_nodes
    ) -> void
    {
        const auto functionNodes =
            DynamicCastFilter<const FunctionBoundNode*>(t_nodes);

        std::for_each(begin(functionNodes), end(functionNodes),
        [&](const FunctionBoundNode* const t_functionNode)
        {
            if (!t_functionNode->GetBody().has_value())
                return;

            t_functionNode->GetSymbol()->BindBody(
                t_functionNode->GetBody().value()
            );
        });
    }

    auto ValidateTypeSizes(
        const Compilation* const t_compilation
    ) -> Expected<void>
    {
        const auto typeSymbols =
            t_compilation->GlobalScope.Unwrap()->CollectSymbolsRecursive<ITypeSymbol>();

        const bool didValidateTypeSizes = std::find_if_not(
            begin(typeSymbols), 
            end  (typeSymbols),
            [](const ITypeSymbol* const t_typeSymbol) -> bool
            {
                auto* const templatableSymbol = dynamic_cast<const ITemplatableSymbol*>(
                    t_typeSymbol
                );
                if (templatableSymbol && templatableSymbol->IsTemplatePlaceholder())
                {
                    return true;
                }

                return t_typeSymbol->GetSizeKind();
            }
        ) == end(typeSymbols);
        ACE_TRY_ASSERT(didValidateTypeSizes);

        return Void{};
    }

    static auto GetOrDefineCopyGlueSymbols(
        const Compilation* const t_compilation,
        ITypeSymbol* const t_typeSymbol
    ) -> FunctionSymbol*
    {
        const auto scope     = t_typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto name = SpecialIdentifier::CreateCopyGlue(
            t_typeSymbol->CreatePartialSignature()
        );

        if (const auto expGlueSymbol = scope->ExclusiveResolveSymbol<FunctionSymbol>(name))
        {
            return expGlueSymbol.Unwrap();
        }

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

        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
            t_typeSymbol->GetWithReference(),
            0
        )).Unwrap();
        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
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

        const auto name = SpecialIdentifier::CreateDropGlue(
            t_typeSymbol->CreatePartialSignature()
        );

        if (const auto expGlueSymbol = scope->ExclusiveResolveSymbol<FunctionSymbol>(name))
        {
            return expGlueSymbol.Unwrap();
        }

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

        Scope::DefineSymbol(std::make_unique<NormalParamVarSymbol>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
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
            return std::nullopt;

        if (t_typeSymbol->GetSizeKind().Unwrap() == TypeSizeKind::Unsized)
            return std::nullopt;

        if (t_typeSymbol->IsReference())
            return std::nullopt;

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
                return;

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

    auto GenerateAndBindGlue(const Compilation* const t_compilation) -> void
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
        StructTypeSymbol* const t_structSymbol,
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialCopyGlueBodyEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialCopyGlueBodyEmitter(ITypeSymbol* const t_typeSymbol)
                : m_TypeSymbol{ t_typeSymbol }
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
        ITypeSymbol* const t_typeSymbol,
        FunctionSymbol* const t_glueSymbol
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
        StructTypeSymbol* const t_structSymbol,
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        if (t_structSymbol->IsTriviallyCopyable())
        {
            return CreateTrivialCopyGlueBody(
                t_compilation,
                t_structSymbol,
                t_glueSymbol
            );
        }

        const auto bodyScope = t_glueSymbol->GetSelfScope()->GetOrCreateChild({});

        const auto paramSymbols = t_glueSymbol->CollectParams();
        const auto selfParamReferenceExprNode = std::make_shared<const StaticVarReferenceExprBoundNode>(
            bodyScope,
            paramSymbols.at(0)
        );
        const auto otherParamReferenceExprNode = std::make_shared<const StaticVarReferenceExprBoundNode>(
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
                bodyScope,
                expOperatorSymbol.Unwrap(),
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
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
                    selfParamReferenceExprNode,
                    t_variableSymbol
                );
                const auto otherParamVarRerefenceExprNode = std::make_shared<const InstanceVarReferenceExprBoundNode>(
                    otherParamReferenceExprNode,
                    t_variableSymbol
                );

                std::vector<std::shared_ptr<const IExprBoundNode>> args{};
                args.push_back(selfParamVarRerefenceExprNode);
                args.push_back(otherParamVarRerefenceExprNode);

                const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                    bodyScope,
                    variableTypeGlueSymbol,
                    args
                );

                const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                    functionCallExprNode
                );

                stmts.push_back(exprStmtNode);
            });
        }

        const auto bodyNode = std::make_shared<const BlockStmtBoundNode>(
            bodyScope->GetParent().value(),
            stmts
        );

        return CreateTransformedAndVerifiedAST(
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
            },
            [&](const std::shared_ptr<const BlockStmtBoundNode>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation->Natives->Void.GetSymbol() }
                );
            }
        ).Unwrap();
    }

    auto CreateDropGlueBody(
        const Compilation* const t_compilation,
        StructTypeSymbol* const t_structSymbol,
        FunctionSymbol* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        if (t_structSymbol->IsTriviallyDroppable())
        {
            return CreateTrivialDropGlueBody(
                t_compilation,
                t_structSymbol,
                t_glueSymbol
            );
        }

        const auto bodyScope = t_glueSymbol->GetSelfScope()->GetOrCreateChild({});

        const auto paramSymbols = t_glueSymbol->CollectParams();
        const auto selfParamReferenceExprNode = std::make_shared<const StaticVarReferenceExprBoundNode>(
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
                bodyScope,
                expOperatorSymbol.Unwrap(),
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
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
                selfParamReferenceExprNode,
                t_variableSymbol
            );

            std::vector<std::shared_ptr<const IExprBoundNode>> args{};
            args.push_back(selfParamVarRerefenceExprNode);

            const auto functionCallExprNode = std::make_shared<const StaticFunctionCallExprBoundNode>(
                bodyScope,
                variableTypeGlueSymbol,
                args
            );

            const auto exprStmtNode = std::make_shared<const ExprStmtBoundNode>(
                functionCallExprNode
            );

            stmts.push_back(exprStmtNode);
        });

        const auto bodyNode = std::make_shared<const BlockStmtBoundNode>(
            bodyScope->GetParent().value(),
            stmts
        );

        return Core::CreateTransformedAndVerifiedAST(
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
            },
            [&](const std::shared_ptr<const BlockStmtBoundNode>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation->Natives->Void.GetSymbol() }
                );
            }
        ).Unwrap();
    }
}
