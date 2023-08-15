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

    static auto DiagnoseMismatchedSelfExprType(
        const std::shared_ptr<const IExprBoundNode>& expr,
        SelfParamVarSymbol* const paramSymbol
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const bool isParamStrongPtr =
            paramSymbol->GetType()->GetWithoutRef()->IsStrongPtr();

        const bool isExprStrongPtr =
            expr->GetTypeInfo().Symbol->GetWithoutRef()->IsStrongPtr();

        if (isParamStrongPtr && !isExprStrongPtr)
        {
            diagnostics.Add(CreateMismatchedSelfExprTypeError(
                expr->GetSrcLocation(),
                paramSymbol
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto InstanceFunctionCallExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const InstanceFunctionCallExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        std::vector<std::shared_ptr<const IExprBoundNode>> convertedArgs = m_Args;
        if (!m_FunctionSymbol->IsError())
        {
            const auto argTypeInfos = m_FunctionSymbol->CollectArgTypeInfos();

            if (m_Args.size() == argTypeInfos.size())
            {
                for (size_t i = 0; i < m_Args.size(); i++)
                {
                    convertedArgs.at(i) = diagnostics.Collect(CreateImplicitlyConverted(
                        m_Args.at(i),
                        argTypeInfos.at(i)
                    ));
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedArgCountError(
                    GetSrcLocation(),
                    m_FunctionSymbol,
                    m_Args.size(),
                    argTypeInfos.size()
                ));
            }

            diagnostics.Collect(DiagnoseMismatchedSelfExprType(
                checkedExpr,
                m_FunctionSymbol->CollectSelfParam().value()
            ));
        }

        std::vector<std::shared_ptr<const IExprBoundNode>> checkedArgs{};
        std::transform(
            begin(convertedArgs),
            end  (convertedArgs),
            back_inserter(checkedArgs),
            [&](const std::shared_ptr<const IExprBoundNode>& arg)
            {
                return diagnostics.Collect(arg->CreateTypeCheckedExpr({}));
            }
        );

        if (
            (checkedExpr == m_Expr) &&
            (checkedArgs == m_Args)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const InstanceFunctionCallExprBoundNode>(
                GetSrcLocation(),
                checkedExpr,
                m_FunctionSymbol,
                checkedArgs
            ),
            std::move(diagnostics),
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

    static auto CreateDerefed(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const IExprBoundNode>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        const bool isRef = typeSymbol->IsRef();
        const bool isStrongPtr = typeSymbol->IsStrongPtr();

        if (isRef)
        {
            return CreateDerefed(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (isStrongPtr)
        {
            return CreateDerefed(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutStrongPtr()
            ));
        }

        return expr;
    }

    static auto CreateDerefedNormalSelfExpr(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const IExprBoundNode>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        const bool isRef = typeSymbol->IsRef();
        const bool isStrongPtr = typeSymbol->IsStrongPtr();

        if (isRef)
        {
            return CreateDerefedNormalSelfExpr(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (isStrongPtr)
        {
            return CreateDerefedNormalSelfExpr(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutStrongPtr()
            ));
        }

        return std::make_shared<const RefExprBoundNode>(
            expr->GetSrcLocation(),
            expr
        );
    }

    static auto CreateDerefedStrongPtrSelfExpr(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const IExprBoundNode>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        const bool isRef = typeSymbol->IsRef();
        const bool isStrongPtr = typeSymbol->IsStrongPtr();

        if (isRef)
        {
            return CreateDerefedNormalSelfExpr(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (isStrongPtr)
        {
            auto* const derefedTypeSymbol = typeSymbol->GetWithoutStrongPtr();

            if (derefedTypeSymbol->IsStrongPtr())
            {
                return CreateDerefedStrongPtrSelfExpr(std::make_shared<const DerefAsExprBoundNode>(
                    expr->GetSrcLocation(),
                    expr,
                    typeSymbol->GetWithoutStrongPtr()
                ));
            }
        }

        return std::make_shared<const RefExprBoundNode>(
            expr->GetSrcLocation(),
            expr
        );
    }

    static auto CreateDerefedSelfExpr(
        const std::shared_ptr<const IExprBoundNode>& expr,
        SelfParamVarSymbol* const paramSymbol
    ) -> std::shared_ptr<const IExprBoundNode>
    {
        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol;
        auto* const paramTypeSymbol = paramSymbol->GetType();

        if (paramTypeSymbol->GetWithoutRef()->IsStrongPtr())
        {
            return CreateDerefedStrongPtrSelfExpr(expr);
        }

        return CreateDerefedNormalSelfExpr(expr);
    }

    auto InstanceFunctionCallExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        std::vector<llvm::Value*> args{};

        const auto selfExpr = CreateDerefedSelfExpr(
            m_Expr,
            m_FunctionSymbol->CollectSelfParam().value()
        );
        auto* const selfType = emitter.GetIRType(selfExpr->GetTypeInfo().Symbol);
        const auto selfEmitResult = selfExpr->Emit(emitter);
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
