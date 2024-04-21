#include "Semas/Exprs/AndExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    AndExprSema::AndExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& lhsExpr,
        const std::shared_ptr<const IExprSema>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto AndExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("AndExprSema", [&]()
        {
            logger.Log("m_LHSExpr", m_LHSExpr);
            logger.Log("m_RHSExpr", m_RHSExpr);
        });
    }

    auto AndExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AndExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto AndExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const AndExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedLHSExpr = diagnostics.Collect(
            CreateImplicitlyConvertedAndTypeChecked(m_LHSExpr, typeInfo)
        );
        const auto checkedRHSExpr = diagnostics.Collect(
            CreateImplicitlyConvertedAndTypeChecked(m_RHSExpr, typeInfo)
        );

        if (
            (checkedLHSExpr == m_LHSExpr) &&
            (checkedRHSExpr == m_RHSExpr)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const AndExprSema>(
                GetSrcLocation(),
                checkedLHSExpr,
                checkedRHSExpr
            ),
            std::move(diagnostics),
        };
    }

    auto AndExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto AndExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const AndExprSema>
    {
        const auto loweredLHSExpr = m_LHSExpr->CreateLoweredExpr({});
        const auto loweredRHSExpr = m_RHSExpr->CreateLoweredExpr({});

        if (
            (loweredLHSExpr == m_LHSExpr) &&
            (loweredRHSExpr == m_RHSExpr)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const AndExprSema>(
            GetSrcLocation(),
            loweredLHSExpr,
            loweredRHSExpr
        )->CreateLowered({});
    }

    auto AndExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto AndExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr);
    }

    auto AndExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        auto* const boolType =
            emitter.GetType(GetCompilation()->GetNatives().Bool.GetSymbol());

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            boolType
        );

        emitter.GetBlock().Builder.CreateStore(
            llvm::ConstantInt::get(boolType, 0),
            allocaInst
        );

        const auto lhsEmitResult = m_LHSExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(lhsEmitResult.Tmps),
            end  (lhsEmitResult.Tmps)
        );

        auto* const lhsLoadInst = emitter.GetBlock().Builder.CreateLoad(
            boolType,
            lhsEmitResult.Value
        );

        auto trueBlock = std::make_unique<EmittingBlock>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        auto endBlock = std::make_unique<EmittingBlock>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        emitter.GetBlock().Builder.CreateCondBr(
            lhsLoadInst,
            trueBlock->Block,
            endBlock->Block
        );

        emitter.SetBlock(std::move(trueBlock));

        const auto rhsEmitResult = m_RHSExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(rhsEmitResult.Tmps),
            end  (rhsEmitResult.Tmps)
        );

        auto* const rhsLoadInst = emitter.GetBlock().Builder.CreateLoad(
            boolType,
            rhsEmitResult.Value
        );

        emitter.GetBlock().Builder.CreateStore(rhsLoadInst, allocaInst);

        emitter.GetBlock().Builder.CreateBr(endBlock->Block);

        emitter.SetBlock(std::move(endBlock));

        return { allocaInst, tmps };
    }

    auto AndExprSema::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R
        };
    }
}
