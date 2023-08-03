#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    StaticFunctionCallExprBoundNode::StaticFunctionCallExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        FunctionSymbol* const functionSymbol,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& args
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_FunctionSymbol{ functionSymbol },
        m_Args{ args }
    {
    }

    auto StaticFunctionCallExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto StaticFunctionCallExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticFunctionCallExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticFunctionCallExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Args);

        return children;
    }

    auto StaticFunctionCallExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const StaticFunctionCallExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const StaticFunctionCallExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetScope(),
            m_FunctionSymbol,
            m_Args
        );
    }

    auto StaticFunctionCallExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const StaticFunctionCallExprBoundNode>>>
    {
        const auto argTypeInfos = m_FunctionSymbol->CollectArgTypeInfos();
        ACE_TRY_ASSERT(m_Args.size() == argTypeInfos.size());

        ACE_TRY(cchConvertedAndCheckedArgs, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Args,
            argTypeInfos
        ));

        if (!cchConvertedAndCheckedArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            m_FunctionSymbol,
            cchConvertedAndCheckedArgs.Value
        ));
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        const auto cchLoweredArgs = TransformCacheableVector(m_Args,
        [&](const std::shared_ptr<const IExprBoundNode>& arg)
        {
            return arg->GetOrCreateLoweredExpr({});
        });

        if (!cchLoweredArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StaticFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            m_FunctionSymbol,
            cchLoweredArgs.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto StaticFunctionCallExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto StaticFunctionCallExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        std::vector<llvm::Value*> args{};
        std::transform(
            begin(m_Args),
            end  (m_Args),
            back_inserter(args),
            [&](const std::shared_ptr<const IExprBoundNode>& arg)
            {
                const auto argEmitResult = arg->Emit(emitter);
                tmps.insert(
                    end(tmps),
                    begin(argEmitResult.Tmps),
                    end  (argEmitResult.Tmps)
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
            return { nullptr, tmps };
        }

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(callInst->getType());
        tmps.emplace_back(allocaInst, m_FunctionSymbol->GetType());

        emitter.GetBlockBuilder().Builder.CreateStore(
            callInst,
            allocaInst
        );

        return { allocaInst, tmps };
    }
    
    auto StaticFunctionCallExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
