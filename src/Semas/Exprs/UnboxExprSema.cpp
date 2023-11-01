#include "Semas/Exprs/UnboxExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    UnboxExprSema::UnboxExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto UnboxExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UnboxExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UnboxExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const UnboxExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const rawExprType = m_Expr->GetTypeInfo().Symbol->GetWithoutRef();

        if (rawExprType->IsDynStrongPtr())
        {
            diagnostics.Add(CreateExpectedNonDynStrongPtrError(
                GetSrcLocation()
            ));
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        if (!rawExprType->IsStrongPtr())
        {
            diagnostics.Add(CreateExpectedStrongPtrError(GetSrcLocation()));
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetCompilation()->GetNatives().strong_ptr_value.GetSymbol(),
            { m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr() }
        );
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        const TypeInfo typeInfo
        {
            functionSymbol->CollectParams().front()->GetType(),
            ValueKind::R,
        };
        const auto checkedExpr = diagnostics.Collect(
            CreateImplicitlyConvertedAndTypeChecked(m_Expr, typeInfo)
        );

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const UnboxExprSema>(
                GetSrcLocation(),
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto UnboxExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto UnboxExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticCallExprSema>
    {
        auto* const rawExprType = m_Expr->GetTypeInfo().Symbol->GetWithoutRef();

        if (rawExprType->IsDynStrongPtr() || !rawExprType->IsStrongPtr())
        {
            return std::make_shared<const StaticCallExprSema>(
                GetSrcLocation(),
                GetScope(),
                GetCompilation()->GetErrorSymbols().GetFunction(),
                std::vector<std::shared_ptr<const IExprSema>>{}
            )->CreateLowered({});
        }

        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetCompilation()->GetNatives().strong_ptr_value.GetSymbol(),
            { loweredExpr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr() }
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

    auto UnboxExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto UnboxExprSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto UnboxExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UnboxExprSema::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr(),
            ValueKind::R,
        };
    }
}
