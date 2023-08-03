#include "BoundNodes/Exprs/FunctionCalls/InstanceFunctionCallExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/RefExprBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceFunctionCallExprBoundNode::InstanceFunctionCallExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        FunctionSymbol* const functionSymbol,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& args
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_FunctionSymbol{ functionSymbol },
        m_Args{ args }
    {
    }

    auto InstanceFunctionCallExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto InstanceFunctionCallExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto InstanceFunctionCallExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto InstanceFunctionCallExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);
        AddChildren(children, m_Args);

        return children;
    }

    auto InstanceFunctionCallExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const InstanceFunctionCallExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const InstanceFunctionCallExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Expr,
            m_FunctionSymbol,
            m_Args
        );
    }

    auto InstanceFunctionCallExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>>
    {
        ACE_TRY(cchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        const auto argTypeInfos = m_FunctionSymbol->CollectArgTypeInfos();
        ACE_TRY_ASSERT(m_Args.size() == argTypeInfos.size());

        ACE_TRY(cchConvertedAndCheckedArgs, CreateImplicitlyConvertedAndTypeCheckedVector(
            m_Args,
            m_FunctionSymbol->CollectArgTypeInfos()
        ));

        if (
            !cchCheckedExpr.IsChanged &&
            !cchConvertedAndCheckedArgs.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchCheckedExpr.Value,
            m_FunctionSymbol,
            cchConvertedAndCheckedArgs.Value
        ));
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto InstanceFunctionCallExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>
    {
        const auto cchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        const auto cchLoweredArgs = TransformCacheableVector(m_Args,
        [&](const std::shared_ptr<const IExprBoundNode>& arg)
        {
            return arg->GetOrCreateLoweredExpr({});
        });

        if (
            !cchLoweredExpr.IsChanged && 
            !cchLoweredArgs.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceFunctionCallExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredExpr.Value,
            m_FunctionSymbol,
            cchLoweredArgs.Value
        )->GetOrCreateLowered({}).Value);
    }


    auto InstanceFunctionCallExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto InstanceFunctionCallExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        std::vector<llvm::Value*> args{};

        const auto expr = [&]() -> std::shared_ptr<const IExprBoundNode>
        {
            if (m_Expr->GetTypeInfo().Symbol->IsRef())
            {
                return m_Expr;
            }

            return std::make_shared<const RefExprBoundNode>(
                DiagnosticBag{},
                m_Expr->GetSrcLocation(),
                m_Expr
            );
        }();

        auto* const selfType = emitter.GetIRType(expr->GetTypeInfo().Symbol);
        
        const auto selfEmitResult = expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(selfEmitResult.Tmps),
            end  (selfEmitResult.Tmps)
        );

        args.push_back(selfEmitResult.Value);

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

    auto InstanceFunctionCallExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
