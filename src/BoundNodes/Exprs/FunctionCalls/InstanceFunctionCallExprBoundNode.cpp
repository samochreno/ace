#include "BoundNodes/Exprs/FunctionCalls/InstanceFunctionCallExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/ReferenceExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceFunctionCallExprBoundNode::InstanceFunctionCallExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        FunctionSymbol* const functionSymbol,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& args
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr },
        m_FunctionSymbol{ functionSymbol },
        m_Args{ args }
    {
    }

    auto InstanceFunctionCallExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto InstanceFunctionCallExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto InstanceFunctionCallExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);
        AddChildren(children, m_Args);

        return children;
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        const auto argTypeInfos = m_FunctionSymbol->CollectArgTypeInfos();
        ACE_TRY_ASSERT(m_Args.size() == argTypeInfos.size());

        ACE_TRY(mchConvertedAndCheckedArgs, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Args,
            m_FunctionSymbol->CollectArgTypeInfos()
        ));

        if (
            !mchCheckedExpr.IsChanged &&
            !mchConvertedAndCheckedArgs.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceFunctionCallExprBoundNode>(
            GetSourceLocation(),
            mchCheckedExpr.Value,
            m_FunctionSymbol,
            mchConvertedAndCheckedArgs.Value
        ));
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        const auto mchLoweredArgs = TransformMaybeChangedVector(m_Args,
        [&](const std::shared_ptr<const IExprBoundNode>& arg)
        {
            return arg->GetOrCreateLoweredExpr({});
        });

        if (
            !mchLoweredExpr.IsChanged && 
            !mchLoweredArgs.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceFunctionCallExprBoundNode>(
            GetSourceLocation(),
            mchLoweredExpr.Value,
            m_FunctionSymbol,
            mchLoweredArgs.Value
        )->GetOrCreateLowered({}).Value);
    }


    auto InstanceFunctionCallExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto InstanceFunctionCallExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        std::vector<llvm::Value*> args{};

        const auto expr = [&]() -> std::shared_ptr<const IExprBoundNode>
        {
            if (m_Expr->GetTypeInfo().Symbol->IsReference())
            {
                return m_Expr;
            }

            return std::make_shared<const ReferenceExprBoundNode>(
                m_Expr->GetSourceLocation(),
                m_Expr
            );
        }();

        auto* const selfType = emitter.GetIRType(expr->GetTypeInfo().Symbol);
        
        const auto selfEmitResult = expr->Emit(emitter);
        temporaries.insert(
            end(temporaries),
            begin(selfEmitResult.Temporaries),
            end  (selfEmitResult.Temporaries)
        );

        args.push_back(selfEmitResult.Value);

        std::transform(
            begin(m_Args),
            end  (m_Args),
            back_inserter(args),
            [&](const std::shared_ptr<const IExprBoundNode>& arg)
            {
                const auto argEmitResult = arg->Emit(emitter);
                temporaries.insert(
                    end(temporaries),
                    begin(argEmitResult.Temporaries),
                    end  (argEmitResult.Temporaries)
                );
                return argEmitResult.Value;
            }
        );

        auto* const callInst = emitter.GetBlockBuilder().Builder.CreateCall(
            emitter.GetFunctionMap().at(m_FunctionSymbol),
            args
        );

        if (callInst->getType()->isVoidTy())
        {
            return { nullptr, temporaries };
        }

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(callInst->getType());
        temporaries.emplace_back(allocaInst, m_FunctionSymbol->GetType());

        emitter.GetBlockBuilder().Builder.CreateStore(
            callInst,
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto InstanceFunctionCallExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
