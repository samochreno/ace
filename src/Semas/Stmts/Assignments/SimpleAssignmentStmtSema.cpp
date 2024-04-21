#include "Semas/Stmts/Assignments/SimpleAssignmentStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ExprDropInfo.hpp"
#include "Symbols/Types/EmittableTypeSymbol.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    SimpleAssignmentStmtSema::SimpleAssignmentStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& lhsExpr,
        const std::shared_ptr<const IExprSema>& rhsExpr
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr }
    {
    }

    auto SimpleAssignmentStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("SimpleAssignmentStmtSema", [&]()
        {
            logger.Log("m_LHSExpr", m_LHSExpr);
            logger.Log("m_RHSExpr", m_RHSExpr);
        });
    }

    auto SimpleAssignmentStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SimpleAssignmentStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto SimpleAssignmentStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const SimpleAssignmentStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const lhsExprTypeSymbol =
            m_LHSExpr->GetTypeInfo().Symbol->GetWithoutRef();

        const auto checkedLHSExpr = diagnostics.Collect(
            CreateImplicitlyConvertedAndTypeChecked(
                m_LHSExpr,
                TypeInfo{ lhsExprTypeSymbol, ValueKind::L }
            )
        );
        const auto checkedRHSExpr = diagnostics.Collect(
            CreateImplicitlyConvertedAndTypeChecked(
                m_RHSExpr,
                TypeInfo{ lhsExprTypeSymbol, ValueKind::R }
            )
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
            std::make_shared<const SimpleAssignmentStmtSema>(
                GetSrcLocation(),
                checkedLHSExpr,
                checkedRHSExpr
            ),
            std::move(diagnostics),
        };
    }

    auto SimpleAssignmentStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto SimpleAssignmentStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const SimpleAssignmentStmtSema>
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

        return std::make_shared<const SimpleAssignmentStmtSema>(
            GetSrcLocation(),
            loweredLHSExpr,
            loweredRHSExpr
        )->CreateLowered({});
    }

    auto SimpleAssignmentStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto SimpleAssignmentStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}
            .Collect(m_LHSExpr)
            .Collect(m_RHSExpr);
    }

    auto SimpleAssignmentStmtSema::Emit(Emitter& emitter) const -> void
    {
        std::vector<ExprDropInfo> tmps{};

        const auto rhsEmitResult = m_RHSExpr.get()->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(rhsEmitResult.Tmps),
            end  (rhsEmitResult.Tmps)
        );

        const auto lhsEmitResult = m_LHSExpr.get()->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(lhsEmitResult.Tmps),
            end  (lhsEmitResult.Tmps)
        );

        emitter.EmitCopy(
            lhsEmitResult.Value,
            rhsEmitResult.Value,
            m_LHSExpr->GetTypeInfo().Symbol
        );

        emitter.EmitDropTmps(tmps);
    }

    auto SimpleAssignmentStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return {};
    }
}
