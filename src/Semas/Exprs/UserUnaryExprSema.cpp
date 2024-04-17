#include "Semas/Exprs/UserUnaryExprSema.hpp"

#include <memory>
#include <vector>

#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    UserUnaryExprSema::UserUnaryExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr,
        FunctionSymbol* const opSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto UserUnaryExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserUnaryExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const UserUnaryExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto convertedExpr = m_Expr;
        if (!m_OpSymbol->IsError())
        {
            const auto argTypeInfos = m_OpSymbol->CollectAllArgTypeInfos();
            ACE_ASSERT(argTypeInfos.size() == 1);

            convertedExpr = diagnostics.Collect(
                CreateImplicitlyConverted(convertedExpr, argTypeInfos.front())
            );
        }

        const auto checkedExpr =
            diagnostics.Collect(convertedExpr->CreateTypeCheckedExpr({}));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const UserUnaryExprSema>(
                GetSrcLocation(),
                checkedExpr,
                m_OpSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto UserUnaryExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto UserUnaryExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticCallExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        std::vector<std::shared_ptr<const IExprSema>> args{};
        if (!m_OpSymbol->IsError())
        {
            args.push_back(loweredExpr);
        }

        return std::make_shared<const StaticCallExprSema>(
            GetSrcLocation(),
            GetScope(),
            m_OpSymbol,
            args
        )->CreateLowered({});
    }

    auto UserUnaryExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto UserUnaryExprSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto UserUnaryExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserUnaryExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
