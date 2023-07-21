#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    StaticFunctionCallExprBoundNode::StaticFunctionCallExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        FunctionSymbol* const functionSymbol,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& args
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_FunctionSymbol{ functionSymbol },
        m_Args{ args }
    {
    }

    auto StaticFunctionCallExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto StaticFunctionCallExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticFunctionCallExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Args);

        return children;
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>>
    {
        const auto argTypeInfos = m_FunctionSymbol->CollectArgTypeInfos();
        ACE_TRY_ASSERT(m_Args.size() == argTypeInfos.size());

        ACE_TRY(mchConvertedAndCheckedArgs, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Args,
            argTypeInfos
        ));

        if (!mchConvertedAndCheckedArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSourceLocation(),
            GetScope(),
            m_FunctionSymbol,
            mchConvertedAndCheckedArgs.Value
        ));
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto mchLoweredArgs = TransformMaybeChangedVector(m_Args,
        [&](const std::shared_ptr<const IExprBoundNode>& arg)
        {
            return arg->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSourceLocation(),
            GetScope(),
            m_FunctionSymbol,
            mchLoweredArgs.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto StaticFunctionCallExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        std::vector<llvm::Value*> args{};
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
                    end(argEmitResult.Temporaries)
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
    
    auto StaticFunctionCallExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
