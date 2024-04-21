#include "Semas/Stmts/CopyStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/EmittableTypeSymbol.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprDropInfo.hpp"

namespace Ace
{
    CopyStmtSema::CopyStmtSema(
        const SrcLocation& srcLocation,
        ISizedTypeSymbol* const typeSymbol,
        const std::shared_ptr<const IExprSema>& srcExpr,
        const std::shared_ptr<const IExprSema>& dstExpr
    ) : m_SrcLocation{ srcLocation },
        m_TypeSymbol{ typeSymbol },
        m_SrcExpr{ srcExpr },
        m_DstExpr{ dstExpr }
    {
    }

    auto CopyStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("CopyStmtSema", [&]()
        {
            logger.Log("m_TypeSymbol", m_TypeSymbol);
            logger.Log("m_SrcExpr", m_SrcExpr);
            logger.Log("m_DstExpr", m_DstExpr);
        });
    }

    auto CopyStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CopyStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_SrcExpr->GetScope();
    }

    auto CopyStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const CopyStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedSrcExpr = diagnostics.Collect(
            m_SrcExpr->CreateTypeCheckedExpr({})
        );
        const auto checkedDstExpr = diagnostics.Collect(
            m_DstExpr->CreateTypeCheckedExpr({})
        );

        auto* const ptrTypeSymbol =
            GetCompilation()->GetNatives().Ptr.GetSymbol();

        if (checkedSrcExpr->GetTypeInfo().Symbol != ptrTypeSymbol)
        {
            diagnostics.Add(CreateExpectedPtrError(
                checkedSrcExpr->GetSrcLocation()
            ));
        }

        if (checkedDstExpr->GetTypeInfo().Symbol != ptrTypeSymbol)
        {
            diagnostics.Add(CreateExpectedPtrError(
                checkedDstExpr->GetSrcLocation()
            ));
        }

        if (
            (checkedSrcExpr == m_SrcExpr) &&
            (checkedDstExpr == m_DstExpr)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const CopyStmtSema>(
                m_SrcLocation,
                m_TypeSymbol,
                checkedSrcExpr,
                checkedDstExpr
            ),
            std::move(diagnostics),
        };
    }

    auto CopyStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto CopyStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const CopyStmtSema>
    {
        const auto loweredSrcExpr = m_SrcExpr->CreateLoweredExpr(context);
        const auto loweredDstExpr = m_DstExpr->CreateLoweredExpr(context);

        if (
            (loweredSrcExpr == m_SrcExpr) &&
            (loweredDstExpr == m_DstExpr)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const CopyStmtSema>(
            m_SrcLocation,
            m_TypeSymbol,
            loweredSrcExpr,
            loweredDstExpr
        );
    }

    auto CopyStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto CopyStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}
            .Collect(m_SrcExpr)
            .Collect(m_DstExpr);
    }

    auto CopyStmtSema::Emit(Emitter& emitter) const -> void
    {
        std::vector<ExprDropInfo> tmps{};

        const auto srcExprEmitResult = m_SrcExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(srcExprEmitResult.Tmps),
            end  (srcExprEmitResult.Tmps)
        );

        const auto dstExprEmitResult = m_DstExpr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(dstExprEmitResult.Tmps),
            end  (dstExprEmitResult.Tmps)
        );

        auto* const instantiatedTypeSymbol =
            emitter.CreateInstantiated<IConcreteTypeSymbol>(m_TypeSymbol);

        auto* const glueSymbol = instantiatedTypeSymbol->GetCopyGlue().value();

        emitter.GetBlock().Builder.CreateCall(
            emitter.GetFunction(glueSymbol),
            { dstExprEmitResult.Value, srcExprEmitResult.Value }
        );
    }

    auto CopyStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return {};
    }
}
