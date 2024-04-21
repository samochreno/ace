#include "Semas/Exprs/UserBinaryExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    UserBinaryExprSema::UserBinaryExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& lhsExpr,
        const std::shared_ptr<const IExprSema>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto UserBinaryExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("UserBinaryExprSema", [&]()
        {
            logger.Log("m_LHSExpr", m_LHSExpr);
            logger.Log("m_RHSExpr", m_RHSExpr);
            logger.Log("m_OpSymbol", m_OpSymbol);
        });
    }

    auto UserBinaryExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserBinaryExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const UserBinaryExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto convertedLHSExpr = m_LHSExpr;
        auto convertedRHSExpr = m_RHSExpr;
        if (!m_OpSymbol->IsError())
        {
            const auto argTypeInfos = m_OpSymbol->CollectAllArgTypeInfos();

            convertedLHSExpr = diagnostics.Collect(
                CreateImplicitlyConverted(m_LHSExpr, argTypeInfos.at(0))
            );
            convertedRHSExpr = diagnostics.Collect(
                CreateImplicitlyConverted(convertedRHSExpr, argTypeInfos.at(1))
            );
        }

        const auto checkedLHSExpr =
            diagnostics.Collect(convertedLHSExpr->CreateTypeCheckedExpr({}));

        const auto checkedRHSExpr =
            diagnostics.Collect(convertedRHSExpr->CreateTypeCheckedExpr({}));

        if (
            (checkedLHSExpr == m_LHSExpr) &&
            (checkedRHSExpr == m_RHSExpr)
            )
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const UserBinaryExprSema>(
                GetSrcLocation(),
                checkedLHSExpr,
                checkedRHSExpr,
                m_OpSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto UserBinaryExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto UserBinaryExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticCallExprSema>
    {
        const auto loweredLHSExpr = m_LHSExpr->CreateLoweredExpr({});
        const auto loweredRHSExpr = m_RHSExpr->CreateLoweredExpr({});

        std::vector<std::shared_ptr<const IExprSema>> args{};
        if (!m_OpSymbol->IsError())
        {
            args.push_back(loweredLHSExpr);
            args.push_back(loweredRHSExpr);
        }

        return std::make_shared<const StaticCallExprSema>(
            GetSrcLocation(),
            GetScope(),
            m_OpSymbol,
            args
        )->CreateLowered({});
    }

    auto UserBinaryExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto UserBinaryExprSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto UserBinaryExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserBinaryExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
