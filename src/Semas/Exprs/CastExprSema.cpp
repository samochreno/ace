#include "Semas/Exprs/CastExprSema.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    CastExprSema::CastExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr,
        ITypeSymbol* const typeSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_TypeSymbol{ typeSymbol }
    {
    }

    auto CastExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("CastExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
            logger.Log("m_TypeSymbol", m_TypeSymbol);
        });
    }

    auto CastExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto CastExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto CastExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const CastExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();
        const TypeInfo targetTypeInfo{ m_TypeSymbol, ValueKind::R };

        diagnostics.Collect(DiagnoseReferenceBinding(
            m_Expr,
            targetTypeInfo
        ));

        const auto convertedExpr = diagnostics.Collect(
            CreateExplicitlyConverted(m_Expr, targetTypeInfo)
        );
        const auto checkedExpr = diagnostics.Collect(
            convertedExpr->CreateTypeCheckedExpr({})
        );

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const CastExprSema>(
                GetSrcLocation(),
                checkedExpr,
                m_TypeSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto CastExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto CastExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const CastExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const CastExprSema>(
            GetSrcLocation(),
            loweredExpr,
            m_TypeSymbol
        )->CreateLowered({});
    }

    auto CastExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto CastExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr).Collect(m_TypeSymbol);
    }

    auto CastExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        return m_Expr->Emit(emitter);
    }

    auto CastExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_TypeSymbol, ValueKind::R };
    }
}
