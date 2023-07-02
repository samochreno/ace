#include "BoundNodes/Exprs/FunctionCalls/InstanceFunctionCallExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/ReferenceExprBoundNode.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceFunctionCallExprBoundNode::InstanceFunctionCallExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        FunctionSymbol* const t_functionSymbol,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& t_args
    ) : m_Expr{ t_expr },
        m_FunctionSymbol{ t_functionSymbol },
        m_Args{ t_args }
    {
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
        const TypeCheckingContext& t_context
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

        const auto returnValue = std::make_shared<const InstanceFunctionCallExprBoundNode>(
            mchCheckedExpr.Value,
            m_FunctionSymbol,
            mchConvertedAndCheckedArgs.Value
        );
        return CreateChanged(returnValue);
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        const auto mchLoweredArgs = TransformMaybeChangedVector(m_Args,
        [&](const std::shared_ptr<const IExprBoundNode>& t_arg)
        {
            return t_arg->GetOrCreateLoweredExpr({});
        });

        if (
            !mchLoweredExpr.IsChanged && 
            !mchLoweredArgs.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const InstanceFunctionCallExprBoundNode>(
            mchLoweredExpr.Value,
            m_FunctionSymbol,
            mchLoweredArgs.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }


    auto InstanceFunctionCallExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto InstanceFunctionCallExprBoundNode::Emit(
        Emitter& t_emitter
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

            return std::make_shared<const ReferenceExprBoundNode>(m_Expr);
        }();

        auto* const selfType =
            t_emitter.GetIRType(expr->GetTypeInfo().Symbol);
        
        const auto selfEmitResult = expr->Emit(t_emitter);
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
            [&](const std::shared_ptr<const IExprBoundNode>& t_arg)
            {
                const auto argEmitResult = t_arg->Emit(t_emitter);
                temporaries.insert(
                    end(temporaries),
                    begin(argEmitResult.Temporaries),
                    end  (argEmitResult.Temporaries)
                );
                return argEmitResult.Value;
            }
        );

        auto* const callInst = t_emitter.GetBlockBuilder().Builder.CreateCall(
            t_emitter.GetFunctionMap().at(m_FunctionSymbol),
            args
        );

        if (callInst->getType()->isVoidTy())
        {
            return { nullptr, temporaries };
        }

        auto* const allocaInst =
            t_emitter.GetBlockBuilder().Builder.CreateAlloca(callInst->getType());
        temporaries.emplace_back(allocaInst, m_FunctionSymbol->GetType());

        t_emitter.GetBlockBuilder().Builder.CreateStore(
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
