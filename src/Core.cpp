#include "Core.hpp"

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Node/Base.hpp"
#include "Node/Module.hpp"
#include "Node/Impl.hpp"
#include "BoundNode/Base.hpp"
#include "BoundNode/Function.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Function.hpp"
#include "Log.hpp"
#include "Utility.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "SpecialIdentifier.hpp"
#include "Emitter.hpp"
#include "Emittable.hpp"
#include "Compilation.hpp"

namespace Ace::Core
{
    auto ParseAST(
        const Compilation& t_compilation,
        const std::string& t_text
    ) -> Expected<std::shared_ptr<const Node::Module>>
    {
        ACE_TRY(tokens, Scanning::Scanner::ScanTokens(
            t_compilation,
            Scanning::Kind::Language,
            t_text
        ));

        ACE_TRY(ast, Parsing::Parser::ParseAST(
            t_compilation,
            std::move(tokens)
        ));

        return ast;
    }

    auto CreateAndDefineSymbols(
        const Compilation& t_compilation,
        const std::vector<const Node::IBase*>& t_nodes
    ) -> Expected<void>
    {
        auto symbolCreatableNodes = DynamicCastFilter<const Node::ISymbolCreatable*>(t_nodes);
        std::sort(begin(symbolCreatableNodes), end(symbolCreatableNodes),
        [](
            const Node::ISymbolCreatable* const t_lhs,
            const Node::ISymbolCreatable* const t_rhs
        )
        {
            const auto lhsCreationOrder = GetSymbolCreationOrder(t_lhs->GetSymbolKind());
            const auto rhsCreationOrder = GetSymbolCreationOrder(t_rhs->GetSymbolKind());

            if (lhsCreationOrder < rhsCreationOrder)
                return true;

            if (lhsCreationOrder > rhsCreationOrder)
                return false;

            const auto lhsKindSpecificCreationOrder = t_lhs->GetSymbolCreationSuborder();
            const auto rhsKindSpecificCreationOrder = t_rhs->GetSymbolCreationSuborder();

            if (lhsKindSpecificCreationOrder < rhsKindSpecificCreationOrder)
                return true;

            if (lhsKindSpecificCreationOrder > rhsKindSpecificCreationOrder)
                return false;

            return false;
        });

        ACE_TRY_VOID(TransformExpectedVector(symbolCreatableNodes,
        [](const Node::ISymbolCreatable* const t_symbolCreatableNode) -> Expected<void>
        {
            ACE_TRY(symbol, Scope::DefineSymbol(t_symbolCreatableNode));
            return ExpectedVoid;
        }));

        return ExpectedVoid;
    }

    auto DefineAssociations(
        const Compilation& t_compilation,
        const std::vector<const Node::IBase*>& t_nodes
    ) -> Expected<void>
    {
        const auto implNodes = DynamicCastFilter<const Node::Impl*>(t_nodes);

        const auto didCreateAssociations = TransformExpectedVector(implNodes,
        [](const Node::Impl* const t_implNode) -> Expected<void>
        {
            ACE_TRY_VOID(t_implNode->DefineAssociations());
            return ExpectedVoid;
        });
        ACE_TRY_ASSERT(didCreateAssociations);

        return ExpectedVoid;
    }

    auto ValidateControlFlow(
        const Compilation& t_compilation,
        const std::vector<const BoundNode::IBase*>& t_nodes
    ) -> Expected<void>
    {
        const auto functionNodes = DynamicCastFilter<const BoundNode::Function*>(t_nodes);

        const bool didControlFlowAnalysisSucceed = std::find_if(
            begin(functionNodes), 
            end  (functionNodes),
            [&](const BoundNode::Function* const t_functionNode)
            {
                if (
                    t_functionNode->GetSymbol()->GetType()->GetUnaliased() == 
                    t_compilation.Natives->Void.GetSymbol()
                    )
                    return false;

                if (!t_functionNode->GetBody().has_value())
                    return false;

                const auto result = t_functionNode->GetBody().value()->IsEndReachableWithoutReturn();
                if (!result)
                    return false;

                return true;
            }
        ) == end(functionNodes);
        ACE_TRY_ASSERT(didControlFlowAnalysisSucceed);

        return ExpectedVoid;
    }

    auto BindFunctionSymbolsBodies(
        const Compilation& t_compilation,
        const std::vector<const BoundNode::IBase*>& t_nodes
    ) -> void
    {
        const auto functionNodes = DynamicCastFilter<const BoundNode::Function*>(t_nodes);
        std::for_each(begin(functionNodes), end(functionNodes),
        [&](const BoundNode::Function* const t_functionNode)
        {
            if (!t_functionNode->GetBody().has_value())
                return;

            t_functionNode->GetSymbol()->BindBody(
                t_functionNode->GetBody().value()
            );
        });
    }

    auto ValidateTypeSizes(
        const Compilation& t_compilation
    ) -> Expected<void>
    {
        const auto typeSymbols = t_compilation.GlobalScope->CollectSymbolsRecursive<Symbol::Type::IBase>();

        const bool didValidateTypeSizes = std::find_if_not(
            begin(typeSymbols), 
            end  (typeSymbols),
            [](const Symbol::Type::IBase* const t_typeSymbol) -> bool
            {
                auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(
                    t_typeSymbol
                );
                if (templatableSymbol && templatableSymbol->IsTemplatePlaceholder())
                    return true;

                return t_typeSymbol->GetSizeKind();
            }
        ) == end(typeSymbols);
        ACE_TRY_ASSERT(didValidateTypeSizes);

        return ExpectedVoid;
    }

    static auto GetOrDefineCopyGlueSymbols(
        const Compilation& t_compilation,
        Symbol::Type::IBase* const t_typeSymbol
    ) -> Symbol::Function*
    {
        const auto scope     = t_typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto name = SpecialIdentifier::CreateCopyGlue(
            t_typeSymbol->CreatePartialSignature()
        );

        if (const auto expGlueSymbol = scope->ExclusiveResolveSymbol<Symbol::Function>(name))
        {
            return expGlueSymbol.Unwrap();
        }

        auto ownedGlueSymbol = std::make_unique<Symbol::Function>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            t_compilation.Natives->Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();

        Scope::DefineSymbol(std::make_unique<Symbol::Variable::Parameter::Normal>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
            t_typeSymbol->GetWithReference(),
            0
        )).Unwrap();
        Scope::DefineSymbol(std::make_unique<Symbol::Variable::Parameter::Normal>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
            t_typeSymbol->GetWithReference(),
            1
        )).Unwrap();

        return glueSymbol;
    }

    static auto GetOrDefineDropGlueSymbols(
        const Compilation& t_compilation,
        Symbol::Type::IBase* const t_typeSymbol
    ) -> Symbol::Function*
    {
        const auto scope     = t_typeSymbol->GetUnaliased()->GetScope();
        const auto selfScope = scope->GetOrCreateChild({});

        const auto name = SpecialIdentifier::CreateDropGlue(
            t_typeSymbol->CreatePartialSignature()
        );

        if (const auto expGlueSymbol = scope->ExclusiveResolveSymbol<Symbol::Function>(name))
        {
            return expGlueSymbol.Unwrap();
        }

        auto ownedGlueSymbol = std::make_unique<Symbol::Function>(
            selfScope,
            name,
            SymbolCategory::Static,
            AccessModifier::Public,
            t_compilation.Natives->Void.GetSymbol()
        );

        auto* const glueSymbol = Scope::DefineSymbol(
            std::move(ownedGlueSymbol)
        ).Unwrap();

        Scope::DefineSymbol(std::make_unique<Symbol::Variable::Parameter::Normal>(
            selfScope,
            SpecialIdentifier::CreateAnonymous(),
            t_typeSymbol->GetWithReference(),
            0
        )).Unwrap();

        return glueSymbol;
    }

    static auto TryDefineGlueSymbols(
        const Compilation& t_compilation,
        Symbol::Type::IBase* const t_typeSymbol,
        const std::function<Symbol::Function*(const Compilation&, Symbol::Type::IBase* const)>& t_getOrDefineGlueSymbols
    ) -> std::optional<Symbol::Function*>
    {
        auto* const templatableSymbol = dynamic_cast<Symbol::ITemplatable*>(
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
        const Compilation& t_compilation,
        const std::function<std::shared_ptr<const IEmittable<void>>(Symbol::Type::IBase* const, Symbol::Function* const)>& t_createGlueBody,
        Symbol::Type::IBase* const t_typeSymbol,
        Symbol::Function* const t_glueSymbol
    ) -> void
    {
        const auto body = t_createGlueBody(t_typeSymbol, t_glueSymbol);
        t_glueSymbol->BindBody(body);
    }

    static auto GenerateAndBindGlue(
        const Compilation& t_compilation,
        const std::function<Symbol::Function*(const Compilation&, Symbol::Type::IBase* const)>& t_getOrDefineGlueSymbols,
        const std::function<std::shared_ptr<const IEmittable<void>>(Symbol::Type::IBase* const, Symbol::Function* const)>& t_createGlueBody,
        const std::function<void(Symbol::Type::IBase* const, Symbol::Function* const)>& t_bindGlue
    ) -> void
    {
        const auto typeSymbols = t_compilation.GlobalScope->CollectSymbolsRecursive<Symbol::Type::IBase>();

        struct TypeGlueSymbolPair
        {
            Symbol::Type::IBase* TypeSymbol{};
            Symbol::Function* FunctionSymbol{};
        };

        std::vector<TypeGlueSymbolPair> typeGlueSymbolPairs{};

        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](Symbol::Type::IBase* const t_typeSymbol)
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

    auto GenerateAndBindGlue(const Compilation& t_compilation) -> void
    {
        GenerateAndBindGlue(
            t_compilation,
            &GetOrDefineCopyGlueSymbols,
            [](
                Symbol::Type::IBase* const t_typeSymbol, 
                Symbol::Function* const t_glueSymbol
            ) 
            { 
                return t_typeSymbol->CreateCopyGlueBody(t_glueSymbol);
            },
            [](
                Symbol::Type::IBase* const t_typeSymbol, 
                Symbol::Function* const t_glueSymbol
            ) 
            { 
                t_typeSymbol->BindCopyGlue(t_glueSymbol);
            }
        );
        GenerateAndBindGlue(
            t_compilation,
            &GetOrDefineDropGlueSymbols,
            [](
                Symbol::Type::IBase* const t_typeSymbol, 
                Symbol::Function* const t_glueSymbol
            ) 
            { 
                return t_typeSymbol->CreateDropGlueBody(t_glueSymbol);
            },
            [](
                Symbol::Type::IBase* const t_typeSymbol, 
                Symbol::Function* const t_glueSymbol
            ) 
            { 
                t_typeSymbol->BindDropGlue(t_glueSymbol);
            }
        );
    }

    static auto CreateTrivialCopyGlueBody(
        const Compilation& t_compilation,
        Symbol::Type::Struct* const t_structSymbol,
        Symbol::Function* const t_glueSymbol
    ) -> std::shared_ptr<const IEmittable<void>>
    {
        class TrivialCopyGlueBodyEmitter : public virtual IEmittable<void>
        {
        public:
            TrivialCopyGlueBodyEmitter(Symbol::Type::IBase* const t_typeSymbol)
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
                
                auto* const selfPtr = t_emitter.EmitLoadArgument(
                    0,
                    ptrType
                );

                auto* const otherPtr = t_emitter.EmitLoadArgument(
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
            Symbol::Type::IBase* m_TypeSymbol{};
        };

        return std::make_shared<const TrivialCopyGlueBodyEmitter>(t_structSymbol);
    }

    static auto CreateTrivialDropGlueBody(
        const Compilation& t_compilation,
        Symbol::Type::IBase* const t_typeSymbol,
        Symbol::Function* const t_glueSymbol
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
        const Compilation& t_compilation,
        Symbol::Type::Struct* const t_structSymbol,
        Symbol::Function* const t_glueSymbol
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

        const auto parameterSymbols = t_glueSymbol->CollectParameters();
        const auto selfParameterReferenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
            bodyScope,
            parameterSymbols.at(0)
        );
        const auto otherParameterReferenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
            bodyScope,
            parameterSymbols.at(1)
        );

        auto operatorName = t_structSymbol->CreateFullyQualifiedName();
        operatorName.Sections.emplace_back(SpecialIdentifier::Operator::Copy);
        const auto expOperatorSymbol = 
            t_compilation.GlobalScope->ResolveStaticSymbol<Symbol::Function>(operatorName);

        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> statements{};
        if (expOperatorSymbol)
        {
            std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> arguments{};
            arguments.push_back(selfParameterReferenceExpressionNode);
            arguments.push_back(otherParameterReferenceExpressionNode);

            const auto functionCallExpressionNode = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
                bodyScope,
                expOperatorSymbol.Unwrap(),
                arguments
            );

            const auto expressionStatementNode = std::make_shared<const BoundNode::Statement::Expression>(
                functionCallExpressionNode
            );

            statements.push_back(expressionStatementNode);
        }
        else
        {
            const auto variableSymbols = t_structSymbol->GetVariables();
            std::for_each(begin(variableSymbols), end(variableSymbols),
            [&](Symbol::Variable::Normal::Instance* const t_variableSymbol)
            {
                auto* const variableTypeSymbol = t_variableSymbol->GetType();
                const auto variableTypeScope = variableTypeSymbol->GetUnaliased()->GetScope();
                auto* const variableTypeGlueSymbol = variableTypeScope->ExclusiveResolveSymbol<Symbol::Function>(
                    SpecialIdentifier::CreateCopyGlue(variableTypeSymbol->CreatePartialSignature())
                ).Unwrap();
                
                const auto selfParameterVariableRerefenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
                    selfParameterReferenceExpressionNode,
                    t_variableSymbol
                );
                const auto otherParameterVariableRerefenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
                    otherParameterReferenceExpressionNode,
                    t_variableSymbol
                );

                std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> arguments{};
                arguments.push_back(selfParameterVariableRerefenceExpressionNode);
                arguments.push_back(otherParameterVariableRerefenceExpressionNode);

                const auto functionCallExpressionNode = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
                    bodyScope,
                    variableTypeGlueSymbol,
                    arguments
                );

                const auto expressionStatementNode = std::make_shared<const BoundNode::Statement::Expression>(
                    functionCallExpressionNode
                );

                statements.push_back(expressionStatementNode);
            });
        }

        const auto bodyNode = std::make_shared<const BoundNode::Statement::Block>(
            bodyScope->GetParent().value(),
            statements
        );

        return CreateTransformedAndVerifiedAST(
            t_compilation,
            bodyNode,
            [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            { 
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation.Natives->Void.GetSymbol() }
                ); 
            },
            [](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateLowered({});
            },
            [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation.Natives->Void.GetSymbol() }
                );
            }
        ).Unwrap();
    }

    auto CreateDropGlueBody(
        const Compilation& t_compilation,
        Symbol::Type::Struct* const t_structSymbol,
        Symbol::Function* const t_glueSymbol
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

        const auto parameterSymbols = t_glueSymbol->CollectParameters();
        const auto selfParameterReferenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Static>(
            bodyScope,
            parameterSymbols.at(0)
        );

        std::vector<std::shared_ptr<const BoundNode::Statement::IBase>> statements{};

        auto operatorName = t_structSymbol->CreateFullyQualifiedName();
        operatorName.Sections.emplace_back(SpecialIdentifier::Operator::Drop);
        const auto expOperatorSymbol = 
            t_compilation.GlobalScope->ResolveStaticSymbol<Symbol::Function>(operatorName);

        if (expOperatorSymbol)
        {
            std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> arguments{};
            arguments.push_back(selfParameterReferenceExpressionNode);

            const auto functionCallExpressionNode = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
                bodyScope,
                expOperatorSymbol.Unwrap(),
                arguments
            );

            const auto expressionStatementNode = std::make_shared<const BoundNode::Statement::Expression>(
                functionCallExpressionNode
            );

            statements.push_back(expressionStatementNode);
        }

        const auto variableSymbols = t_structSymbol->GetVariables();
        std::for_each(rbegin(variableSymbols), rend(variableSymbols),
        [&](Symbol::Variable::Normal::Instance* const t_variableSymbol)
        {
            auto* const variableTypeSymbol = t_variableSymbol->GetType();
            const auto variableTypeScope = variableTypeSymbol->GetUnaliased()->GetScope();
            auto* const variableTypeGlueSymbol = variableTypeScope->ExclusiveResolveSymbol<Symbol::Function>(
                SpecialIdentifier::CreateDropGlue(variableTypeSymbol->CreatePartialSignature())
            ).Unwrap();
            
            const auto selfParameterVariableRerefenceExpressionNode = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
                selfParameterReferenceExpressionNode,
                t_variableSymbol
            );

            std::vector<std::shared_ptr<const BoundNode::Expression::IBase>> arguments{};
            arguments.push_back(selfParameterVariableRerefenceExpressionNode);

            const auto functionCallExpressionNode = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
                bodyScope,
                variableTypeGlueSymbol,
                arguments
            );

            const auto expressionStatementNode = std::make_shared<const BoundNode::Statement::Expression>(
                functionCallExpressionNode
            );

            statements.push_back(expressionStatementNode);
        });

        const auto bodyNode = std::make_shared<const BoundNode::Statement::Block>(
            bodyScope->GetParent().value(),
            statements
        );

        return Core::CreateTransformedAndVerifiedAST(
            t_compilation,
            bodyNode,
            [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            { 
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation.Natives->Void.GetSymbol() }
                ); 
            },
            [](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateLowered({});
            },
            [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_bodyNode)
            {
                return t_bodyNode->GetOrCreateTypeChecked(
                    { t_compilation.Natives->Void.GetSymbol() }
                );
            }
        ).Unwrap();
    }
}
