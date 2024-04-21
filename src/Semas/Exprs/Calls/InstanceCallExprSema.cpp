#include "Semas/Exprs/Calls/InstanceCallExprSema.hpp"

#include <memory>
#include <vector>

#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/RefExprSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/CallableSymbol.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceCallExprSema::InstanceCallExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr,
        ICallableSymbol* const callableSymbol,
        const std::vector<std::shared_ptr<const IExprSema>>& args
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_CallableSymbol{ callableSymbol },
        m_Args{ args }
    {
    }

    auto InstanceCallExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("InstanceCallExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
            logger.Log("m_CallableSymbol", m_CallableSymbol);
            logger.Log("m_Args", m_Args);
        });
    }

    auto InstanceCallExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto InstanceCallExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    static auto IsFunctionNotDynDispatchable(
        const std::shared_ptr<const IExprSema>& expr,
        ICallableSymbol* const callableSymbol
    ) -> bool
    {
        auto* const prototypeSymbol =
            dynamic_cast<PrototypeSymbol*>(callableSymbol->GetUnaliased());
        if (!prototypeSymbol)
        {
            return false;
        }

        const bool isSelfDyn =
            expr->GetTypeInfo().Symbol->GetWithoutRef()->IsDynStrongPtr();
        if (!isSelfDyn)
        {
            return false;
        }

        return !prototypeSymbol->IsDynDispatchable();
    }

    static auto DiagnoseFunctionNotDynDispatchable(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr,
        ICallableSymbol* const callableSymbol
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (IsFunctionNotDynDispatchable(expr, callableSymbol))
        {
            diagnostics.Add(CreateFunctionNotDynDispatchableError(srcLocation));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseMismatchedSelfExprType(
        const std::shared_ptr<const IExprSema>& expr,
        SelfParamVarSymbol* const paramSymbol
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const bool isParamStrongPtr =
            paramSymbol->GetType()->GetWithoutRef()->IsAnyStrongPtr();

        const bool isExprStrongPtr =
            expr->GetTypeInfo().Symbol->GetWithoutRef()->IsAnyStrongPtr();

        if (isParamStrongPtr && !isExprStrongPtr)
        {
            diagnostics.Add(CreateMismatchedSelfExprTypeError(
                expr->GetSrcLocation(),
                paramSymbol
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto InstanceCallExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const InstanceCallExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        std::vector<std::shared_ptr<const IExprSema>> convertedArgs = m_Args;
        if (!m_CallableSymbol->IsError())
        {
            const auto argTypeInfos = m_CallableSymbol->CollectArgTypeInfos();

            if (m_Args.size() == argTypeInfos.size())
            {
                for (size_t i = 0; i < m_Args.size(); i++)
                {
                    const auto& arg = m_Args.at(i);
                    const auto& argTypeInfo = argTypeInfos.at(i);

                    convertedArgs.at(i) = diagnostics.Collect(
                        CreateImplicitlyConverted(arg, argTypeInfo)
                    );
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedArgCountError(
                    GetSrcLocation(),
                    m_CallableSymbol,
                    m_Args.size(),
                    argTypeInfos.size()
                ));
            }

            diagnostics.Collect(DiagnoseMismatchedSelfExprType(
                checkedExpr,
                m_CallableSymbol->CollectSelfParam().value()
            ));

            diagnostics.Collect(DiagnoseFunctionNotDynDispatchable(
                GetSrcLocation(),
                checkedExpr,
                m_CallableSymbol
            ));
        }

        std::vector<std::shared_ptr<const IExprSema>> checkedArgs{};
        std::transform(
            begin(convertedArgs),
            end  (convertedArgs),
            back_inserter(checkedArgs),
            [&](const std::shared_ptr<const IExprSema>& arg)
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
            std::make_shared<const InstanceCallExprSema>(
                GetSrcLocation(),
                checkedExpr,
                m_CallableSymbol,
                checkedArgs
            ),
            std::move(diagnostics),
        };
    }

    auto InstanceCallExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto InstanceCallExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const InstanceCallExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        std::vector<std::shared_ptr<const IExprSema>> loweredArgs{};
        if (!m_CallableSymbol->IsError())
        {
            std::transform(
                begin(m_Args),
                end  (m_Args),
                back_inserter(loweredArgs),
                [&](const std::shared_ptr<const IExprSema>& arg)
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

        return std::make_shared<const InstanceCallExprSema>(
            GetSrcLocation(),
            loweredExpr,
            m_CallableSymbol,
            loweredArgs
        )->CreateLowered({});
    }

    auto InstanceCallExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    static auto CreateDerefed(
        const std::shared_ptr<const IExprSema>& expr
    ) -> std::shared_ptr<const IExprSema>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        if (typeSymbol->IsRef())
        {
            return CreateDerefed(std::make_shared<const DerefAsExprSema>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (typeSymbol->IsAnyStrongPtr())
        {
            return CreateDerefed(std::make_shared<const DerefAsExprSema>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutStrongPtr()
            ));
        }

        return expr;
    }

    static auto CreateDerefedNormalSelfExpr(
        const std::shared_ptr<const IExprSema>& expr
    ) -> std::shared_ptr<const IExprSema>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        if (typeSymbol->IsRef())
        {
            return CreateDerefedNormalSelfExpr(std::make_shared<const DerefAsExprSema>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (typeSymbol->IsAnyStrongPtr())
        {
            return CreateDerefedNormalSelfExpr(std::make_shared<const DerefAsExprSema>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutStrongPtr()
            ));
        }

        return std::make_shared<const RefExprSema>(
            expr->GetSrcLocation(),
            expr
        );
    }

    static auto CreateDerefedStrongPtrSelfExpr(
        const std::shared_ptr<const IExprSema>& expr
    ) -> std::shared_ptr<const IExprSema>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        if (typeSymbol->IsRef())
        {
            return CreateDerefedNormalSelfExpr(std::make_shared<const DerefAsExprSema>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (typeSymbol->IsAnyStrongPtr())
        {
            auto* const derefedTypeSymbol = typeSymbol->GetWithoutStrongPtr();

            if (derefedTypeSymbol->IsAnyStrongPtr())
            {
                return CreateDerefedStrongPtrSelfExpr(std::make_shared<const DerefAsExprSema>(
                    expr->GetSrcLocation(),
                    expr,
                    typeSymbol->GetWithoutStrongPtr()
                ));
            }
        }

        return std::make_shared<const RefExprSema>(
            expr->GetSrcLocation(),
            expr
        );
    }

    static auto CreateDerefedSelfExpr(
        const std::shared_ptr<const IExprSema>& expr,
        SelfParamVarSymbol* const paramSymbol
    ) -> std::shared_ptr<const IExprSema>
    {
        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol;
        auto* const paramTypeSymbol = paramSymbol->GetType();

        if (paramTypeSymbol->GetWithoutRef()->IsAnyStrongPtr())
        {
            return CreateDerefedStrongPtrSelfExpr(expr);
        }

        return CreateDerefedNormalSelfExpr(expr);
    }

    auto InstanceCallExprSema::CollectMonos() const -> MonoCollector 
    {
        return MonoCollector{}
            .Collect(m_Expr)
            .Collect(m_CallableSymbol)
            .Collect(m_Args);
    }

    auto InstanceCallExprSema::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        std::vector<llvm::Value*> args{};

        const auto selfExpr = CreateDerefedSelfExpr(
            m_Expr,
            m_CallableSymbol->CollectSelfParam().value()
        );
        auto* const selfType = emitter.GetType(selfExpr->GetTypeInfo().Symbol);
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
            [&](const std::shared_ptr<const IExprSema>& arg)
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

        auto* const callInst = emitter.EmitCall(m_CallableSymbol, args);

        if (callInst->getType()->isVoidTy())
        {
            return { nullptr, tmps };
        }

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            callInst->getType()
        );
        tmps.emplace_back(allocaInst, m_CallableSymbol->GetType());

        emitter.GetBlock().Builder.CreateStore(
            callInst,
            allocaInst
        );

        return { allocaInst, tmps };
    }

    auto InstanceCallExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_CallableSymbol->GetType(), ValueKind::R };
    }
}
