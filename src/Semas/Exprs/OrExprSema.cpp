#include "Semas/Exprs/OrExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    OrExprSema::OrExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& lhsExpr,
        const std::shared_ptr<const IExprSema>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto OrExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto OrExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto OrExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const OrExprSema>>
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
            std::make_shared<const OrExprSema>(
                GetSrcLocation(),
                checkedLHSExpr,
                checkedRHSExpr
            ),
            std::move(diagnostics),
        };
    }

    auto OrExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto OrExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const OrExprSema>
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

        return std::make_shared<const OrExprSema>(
            GetSrcLocation(),
            loweredLHSExpr,
            loweredRHSExpr
        )->CreateLowered({});
    }

    auto OrExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto OrExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr);
    }

    auto OrExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        auto* const boolType =
            emitter.GetType(GetCompilation()->GetNatives().Bool.GetSymbol());

        auto* const allocaInst =
            emitter.GetBlock().Builder.CreateAlloca(boolType);

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

        emitter.GetBlock().Builder.CreateStore(lhsLoadInst, allocaInst);

        auto falseBlock = std::make_unique<EmittingBlock>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        auto endBlock = std::make_unique<EmittingBlock>(
            emitter.GetContext(),
            emitter.GetFunction()
        );

        emitter.GetBlock().Builder.CreateCondBr(
            lhsLoadInst,
            endBlock->Block,
            falseBlock->Block
        );

        emitter.SetBlock(std::move(falseBlock));

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

    auto OrExprSema::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R 
        };
    }
}
