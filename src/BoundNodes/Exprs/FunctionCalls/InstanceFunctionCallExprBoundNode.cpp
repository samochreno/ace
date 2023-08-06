#include "BoundNodes/Exprs/FunctionCalls/InstanceFunctionCallExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/RefExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceFunctionCallExprBoundNode::InstanceFunctionCallExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        FunctionSymbol* const functionSymbol,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& args
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_FunctionSymbol{ functionSymbol },
        m_Args{ args }
    {
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

    auto InstanceFunctionCallExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto dgnCheckedExpr = m_Expr->CreateTypeCheckedExpr({});
        diagnostics.Add(dgnCheckedExpr);

        std::vector<std::shared_ptr<const IExprBoundNode>> convertedArgs = m_Args;
        if (!m_FunctionSymbol->IsError())
        {
            const auto argTypeInfos = m_FunctionSymbol->CollectArgTypeInfos();

            if (m_Args.size() != argTypeInfos.size())
            {
                diagnostics.Add(CreateUnexpectedArgCountError(
                    GetSrcLocation(),
                    m_FunctionSymbol,
                    m_Args.size(),
                    argTypeInfos.size()
                ));
            }

            for (size_t i = 0; i < m_Args.size(); i++)
            {
                const auto dgnConvertedArg = CreateImplicitlyConverted(
                    m_Args.at(i),
                    argTypeInfos.at(i)
                );
                diagnostics.Add(dgnConvertedArg);
                convertedArgs.at(i) = dgnConvertedArg.Unwrap();
            }
        }

        std::vector<std::shared_ptr<const IExprBoundNode>> checkedArgs{};
        std::transform(
            begin(convertedArgs),
            end  (convertedArgs),
            back_inserter(checkedArgs),
            [&](const std::shared_ptr<const IExprBoundNode>& arg)
            {
                const auto dgnCheckedArg = arg->CreateTypeCheckedExpr({});
                diagnostics.Add(dgnCheckedArg);
                return dgnCheckedArg.Unwrap();
            }
        );

        if (
            (dgnCheckedExpr.Unwrap() == m_Expr) &&
            (checkedArgs == m_Args)
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const InstanceFunctionCallExprBoundNode>(
                GetSrcLocation(),
                dgnCheckedExpr.Unwrap(),
                m_FunctionSymbol,
                checkedArgs
            ),
            diagnostics,
        };
    }

    auto InstanceFunctionCallExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto InstanceFunctionCallExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const InstanceFunctionCallExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        std::vector<std::shared_ptr<const IExprBoundNode>> loweredArgs{};
        if (!m_FunctionSymbol->IsError())
        {
            std::transform(
                begin(m_Args),
                end  (m_Args),
                back_inserter(loweredArgs),
                [&](const std::shared_ptr<const IExprBoundNode>& arg)
                {
                    return arg->CreateLoweredExpr({});
                }
            );
        }

        if (
            (loweredExpr == m_Expr) &&
            (loweredArgs == m_Args)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const InstanceFunctionCallExprBoundNode>(
            GetSrcLocation(),
            loweredExpr,
            m_FunctionSymbol,
            loweredArgs
        )->CreateLowered({});
    }


    auto InstanceFunctionCallExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
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
