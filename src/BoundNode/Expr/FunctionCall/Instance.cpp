#include "BoundNode/Expr/FunctionCall/Instance.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Expr//Reference.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expr::FunctionCall
{
    Instance::Instance(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        Symbol::Function* const t_functionSymbol,
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_args
    ) : m_Expr{ t_expr },
        m_FunctionSymbol{ t_functionSymbol },
        m_Args{ t_args }
    {
    }

    auto Instance::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto Instance::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expr);
        AddChildren(children, m_Args);

        return children;
    }

    auto Instance::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Instance>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        const auto argTypeInfos =
            m_FunctionSymbol->CollectArgTypeInfos();
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

        const auto returnValue = std::make_shared<const BoundNode::Expr::FunctionCall::Instance>(
            mchCheckedExpr.Value,
            m_FunctionSymbol,
            mchConvertedAndCheckedArgs.Value
        );
        return CreateChanged(returnValue);
    }

    auto Instance::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Instance::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Instance>>
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        const auto mchLoweredArgs = TransformMaybeChangedVector(m_Args,
        [&](const std::shared_ptr<const BoundNode::Expr::IBase>& t_arg)
        {
            return t_arg->GetOrCreateLoweredExpr({});
        });

        if (
            !mchLoweredExpr.IsChanged && 
            !mchLoweredArgs.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expr::FunctionCall::Instance>(
            mchLoweredExpr.Value,
            m_FunctionSymbol,
            mchLoweredArgs.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }


    auto Instance::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Instance::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        std::vector<llvm::Value*> args{};

        const auto expr = [&]() -> std::shared_ptr<const BoundNode::Expr::IBase>
        {
            if (m_Expr->GetTypeInfo().Symbol->IsReference())
            {
                return m_Expr;
            }

            return std::make_shared<const BoundNode::Expr::Reference>(
                m_Expr
            );
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
            [&](const std::shared_ptr<const BoundNode::Expr::IBase>& t_arg)
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

    auto Instance::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
