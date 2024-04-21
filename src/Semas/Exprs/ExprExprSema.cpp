#include "Semas/Exprs/ExprExprSema.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    ExprExprSema::ExprExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto ExprExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("ExprExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
        });
    }

    auto ExprExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExprExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ExprExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const ExprExprSema>(
                GetSrcLocation(),
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto ExprExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto ExprExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ExprExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const ExprExprSema>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto ExprExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto ExprExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr);
    }

    auto ExprExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        return m_Expr->Emit(emitter);
    }

    auto ExprExprSema::GetTypeInfo() const -> TypeInfo
    {
        return m_Expr->GetTypeInfo();
    }
}
