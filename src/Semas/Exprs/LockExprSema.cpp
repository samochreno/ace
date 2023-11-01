
#include "Semas/Exprs/LockExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    LockExprSema::LockExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto LockExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LockExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    static auto GetRawExprTypeSymbol(
        const std::shared_ptr<const IExprSema>& expr
    ) -> ITypeSymbol*
    {
        return expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutWeakPtr();
    }

    static auto GetLockSymbol(
        ITypeSymbol* const rawExprTypeSymbol
    ) -> FunctionSymbol*
    {
        auto* const sizedRawExprTypeSymbol =
            dynamic_cast<ISizedTypeSymbol*>(rawExprTypeSymbol->GetUnaliased());

        const auto& natives = rawExprTypeSymbol->GetCompilation()->GetNatives();

        return (sizedRawExprTypeSymbol ?
            natives.weak_ptr_lock :
            natives.weak_ptr_lock_dyn
        ).GetSymbol();
    }

    auto LockExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const LockExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const rawExprTypeSymbol = GetRawExprTypeSymbol(m_Expr);

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetLockSymbol(rawExprTypeSymbol),
            { rawExprTypeSymbol }
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
            std::make_shared<const LockExprSema>(GetSrcLocation(), checkedExpr),
            std::move(diagnostics),
        };
    }
    
    auto LockExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto LockExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticCallExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        auto* const rawExprTypeSymbol = GetRawExprTypeSymbol(loweredExpr);

        auto* const symbol = Scope::ForceCollectGenericInstance(
            GetLockSymbol(rawExprTypeSymbol),
            { rawExprTypeSymbol }
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

    auto LockExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto LockExprSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto LockExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto LockExprSema::GetTypeInfo() const -> TypeInfo
    {
        return
        { 
            GetRawExprTypeSymbol(m_Expr)->GetWithAutoStrongPtr(),
            ValueKind::R
        };
    }
}
