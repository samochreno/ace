#include "BoundNode/Expr/FunctionCall/Static.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/Base.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expr::FunctionCall
{
    Static::Static(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Function* const t_functionSymbol,
        const std::vector<std::shared_ptr<const BoundNode::Expr::IBase>>& t_args
    ) : m_Scope{ t_scope },
        m_FunctionSymbol{ t_functionSymbol },
        m_Args{ t_args }
    {
    }

    auto Static::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Static::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Args);

        return children;
    }

    auto Static::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Static>>>
    {
        const auto argTypeInfos =
            m_FunctionSymbol->CollectArgTypeInfos();
        ACE_TRY_ASSERT(m_Args.size() == argTypeInfos.size());

        ACE_TRY(mchConvertedAndCheckedArgs, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Args,
            argTypeInfos
        ));

        if (!mchConvertedAndCheckedArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::FunctionCall::Static>(
            m_Scope,
            m_FunctionSymbol,
            mchConvertedAndCheckedArgs.Value
        );
        return CreateChanged(returnValue);
    }

    auto Static::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Static::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::FunctionCall::Static>>
    {
        const auto mchLoweredArgs = TransformMaybeChangedVector(m_Args,
        [&](const std::shared_ptr<const BoundNode::Expr::IBase>& t_arg)
        {
            return t_arg->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::FunctionCall::Static>(
            m_Scope,
            m_FunctionSymbol,
            mchLoweredArgs.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Static::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Static::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        std::vector<llvm::Value*> args{};
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
                    end(argEmitResult.Temporaries)
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
    
    auto Static::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
