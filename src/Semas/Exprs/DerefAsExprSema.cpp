#include "Semas/Exprs/DerefAsExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropInfo.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    DerefAsExprSema::DerefAsExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr,
        ITypeSymbol* const typeSymbol
    ) : m_SrcLocation{ srcLocation },
        m_TypeSymbol{ typeSymbol },
        m_Expr{ expr }
    {
    }

    auto DerefAsExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefAsExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const DerefAsExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const typeSymbol =
            m_Expr->GetTypeInfo().Symbol->GetUnaliasedType();

        const bool isRef = typeSymbol->IsRef();
        const bool isPtr =
            typeSymbol == GetCompilation()->GetNatives().Ptr.GetSymbol();

        if (!isRef && !isPtr)
        {
            diagnostics.Add(CreateExpectedDerefableExprError(GetSrcLocation()));
        }

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const DerefAsExprSema>(
                GetSrcLocation(),
                checkedExpr,
                m_TypeSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto DerefAsExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto DerefAsExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const DerefAsExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const DerefAsExprSema>(
            GetSrcLocation(),
            loweredExpr,
            m_TypeSymbol
        )->CreateLowered({});
    }

    auto DerefAsExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto DerefAsExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr).Collect(m_TypeSymbol);
    }

    auto DerefAsExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );

        auto* const loadInst = emitter.GetBlock().Builder.CreateLoad(
            emitter.GetPtrType(),
            exprEmitResult.Value
        );

        return { loadInst, tmps };
    }

    auto DerefAsExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_TypeSymbol, ValueKind::R };
    }
}
