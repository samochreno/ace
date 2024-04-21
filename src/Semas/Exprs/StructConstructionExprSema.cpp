#include "Semas/Exprs/StructConstructionExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "SemaLogger.hpp"
#include "Symbols/Types/EmittableTypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    StructConstructionExprSema::StructConstructionExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        StructTypeSymbol* const structSymbol,
        const std::vector<StructConstructionExprSemaArg>& args
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_StructSymbol{ structSymbol },
        m_Args{ args }
    {
    }

    auto StructConstructionExprSema::Log(SemaLogger &logger) const -> void
    {
        logger.Log("StructConstructionExprSema", [&]()
        {
            logger.Log("m_StructSymbol", m_StructSymbol);

            std::for_each(begin(m_Args), end(m_Args),
            [&](const StructConstructionExprSemaArg& arg)
            {
                logger.Log(arg.Symbol->GetName().String, arg.Value);
            });
        });
    }

    auto StructConstructionExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StructConstructionExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StructConstructionExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StructConstructionExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<StructConstructionExprSemaArg> checkedArgs{};
        std::transform(
            begin(m_Args),
            end  (m_Args),
            back_inserter(checkedArgs),
            [&](const StructConstructionExprSemaArg& arg)
            {
                const auto checkedValue =
                    diagnostics.Collect(arg.Value->CreateTypeCheckedExpr({}));

                return StructConstructionExprSemaArg
                {
                    arg.Symbol,
                    checkedValue,
                };
            }
        );

        if (checkedArgs == m_Args)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const StructConstructionExprSema>(
                GetSrcLocation(),
                GetScope(),
                m_StructSymbol,
                checkedArgs
            ),
            std::move(diagnostics),
        };
    }

    auto StructConstructionExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto StructConstructionExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StructConstructionExprSema>
    {
        std::vector<StructConstructionExprSemaArg> loweredArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(loweredArgs),
        [&](const StructConstructionExprSemaArg& arg)
        {
            const auto loweredValue = arg.Value->CreateLoweredExpr(context);
            return StructConstructionExprSemaArg{ arg.Symbol, loweredValue };
        });

        if (loweredArgs == m_Args)
        {
            return shared_from_this();
        }

        return std::make_shared<const StructConstructionExprSema>(
            GetSrcLocation(),
            GetScope(),
            m_StructSymbol,
            loweredArgs
        )->CreateLowered({});
    }

    auto StructConstructionExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto StructConstructionExprSema::CollectMonos() const -> MonoCollector
    {
        MonoCollector collector{};

        collector.Collect(m_StructSymbol);

        std::for_each(begin(m_Args), end(m_Args),
        [&](const StructConstructionExprSemaArg& arg)
        {
            collector.Collect(arg.Value);
        });

        return collector;
    }

    auto StructConstructionExprSema::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        auto* const structType = emitter.GetType(m_StructSymbol);

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            structType
        );
        tmps.emplace_back(allocaInst, m_StructSymbol);

        std::for_each(begin(m_Args), end(m_Args),
        [&](const StructConstructionExprSemaArg& arg)
        {
            auto* const argTypeSymbol = arg.Value->GetTypeInfo().Symbol;
            auto* const argType = emitter.GetType(argTypeSymbol);

            const auto varIndex = arg.Symbol->GetIndex();

            auto* const int32Type =
                llvm::Type::getInt32Ty(emitter.GetContext());

            auto* const elementPtr = emitter.GetBlock().Builder.CreateStructGEP(
                structType,
                allocaInst,
                varIndex
            );

            const auto argEmitResult = arg.Value->Emit(emitter);
            tmps.insert(
                end(tmps),
                begin(argEmitResult.Tmps),
                end  (argEmitResult.Tmps)
            );

            emitter.EmitCopy(elementPtr, argEmitResult.Value, argTypeSymbol);
        });
        
        return { allocaInst, tmps };
    }

    auto StructConstructionExprSema::GetTypeInfo() const -> TypeInfo
    {
        return TypeInfo{ m_StructSymbol, ValueKind::R };
    }
}
