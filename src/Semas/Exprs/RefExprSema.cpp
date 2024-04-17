#include "Semas/Exprs/RefExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    RefExprSema::RefExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto RefExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto RefExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto RefExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const RefExprSema>>
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
            std::make_shared<const RefExprSema>(
                GetSrcLocation(),
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto RefExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto RefExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const RefExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const RefExprSema>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto RefExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto RefExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr);
    }

    auto RefExprSema::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        ); 

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            exprEmitResult.Value->getType()
        );
        auto* const exprTypeSymbol = dynamic_cast<ISizedTypeSymbol*>(
            m_Expr->GetTypeInfo().Symbol->GetWithRef()
        );
        tmps.emplace_back(allocaInst, exprTypeSymbol);

        emitter.GetBlock().Builder.CreateStore(
            exprEmitResult.Value,
            allocaInst
        );

        return { allocaInst, tmps };
    }

    auto RefExprSema::GetTypeInfo() const -> TypeInfo
    {
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        const auto scope = typeSymbol->GetScope();

        return { typeSymbol->GetWithRef(), ValueKind::R };
    }

    auto RefExprSema::GetExpr() const -> std::shared_ptr<const IExprSema>
    {
        return m_Expr;
    }
}
