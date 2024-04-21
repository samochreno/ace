#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"

#include <memory>
#include <vector>

#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/CallableSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"
#include "SemaLogger.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    StaticCallExprSema::StaticCallExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ICallableSymbol* const callableSymbol,
        const std::vector<std::shared_ptr<const IExprSema>>& args
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_CallableSymbol{ callableSymbol },
        m_Args{ args }
    {
    }

    auto StaticCallExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("StaticCallExprSema", [&]()
        {
            logger.Log("m_CallableSymbol", m_CallableSymbol);
            logger.Log("m_Args", m_Args);
        });
    }

    auto StaticCallExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticCallExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticCallExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StaticCallExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

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

        if (checkedArgs == m_Args)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const StaticCallExprSema>(
                GetSrcLocation(),
                GetScope(),
                m_CallableSymbol,
                checkedArgs
            ),
            std::move(diagnostics),
        };
    }

    auto StaticCallExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto StaticCallExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticCallExprSema>
    {
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

        if (loweredArgs == m_Args)
        {
            return shared_from_this();
        }

        return std::make_shared<const StaticCallExprSema>(
            GetSrcLocation(),
            GetScope(),
            m_CallableSymbol,
            loweredArgs
        )->CreateLowered({});
    }

    auto StaticCallExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto StaticCallExprSema::CollectMonos() const -> MonoCollector 
    {
        return MonoCollector{}
            .Collect(m_CallableSymbol)
            .Collect(m_Args);
    }

    auto StaticCallExprSema::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        std::vector<llvm::Value*> args{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(args),
        [&](const std::shared_ptr<const IExprSema>& arg)
        {
            const auto argEmitResult = arg->Emit(emitter);
            tmps.insert(
                end(tmps),
                begin(argEmitResult.Tmps),
                end  (argEmitResult.Tmps)
            );
            return argEmitResult.Value;
        });


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
    
    auto StaticCallExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_CallableSymbol->GetType(), ValueKind::R };
    }
}
