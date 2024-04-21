#include "Semas/Exprs/BoxExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    BoxExprSema::BoxExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto BoxExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("BoxExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
        });
    }

    auto BoxExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto BoxExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto BoxExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const BoxExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const rawExprType = m_Expr->GetTypeInfo().Symbol->GetWithoutRef();

        auto checkedExpr = m_Expr;
        if (dynamic_cast<ISizedTypeSymbol*>(rawExprType->GetUnaliased()))
        {
            auto* const symbol = Scope::ForceCollectGenericInstance(
                GetCompilation()->GetNatives().strong_ptr_new.GetSymbol(),
                { rawExprType }
            );
            auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
            ACE_ASSERT(functionSymbol);

            const TypeInfo typeInfo
            {
                functionSymbol->CollectParams().front()->GetType(),
                ValueKind::R,
            };
            checkedExpr = diagnostics.Collect(
                CreateImplicitlyConvertedAndTypeChecked(m_Expr, typeInfo)
            );
        }
        else
        {
            diagnostics.Add(CreateExpectedSizedExprError(m_Expr));
        }


        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const BoxExprSema>(GetSrcLocation(), checkedExpr),
            std::move(diagnostics),
        };
    }
    
    auto BoxExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto BoxExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticCallExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetCompilation()->GetNatives().strong_ptr_new.GetSymbol(),
            { loweredExpr->GetTypeInfo().Symbol->GetWithoutRef() }
        );
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        return std::make_shared<const StaticCallExprSema>(
            GetSrcLocation(),
            GetScope(),
            functionSymbol,
            std::vector{ loweredExpr }
        )->CreateLowered({});
    }

    auto BoxExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto BoxExprSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto BoxExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto BoxExprSema::GetTypeInfo() const -> TypeInfo
    {
        return
        { 
            m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithStrongPtr(),
            ValueKind::R
        };
    }
}
