#include "Semas/Exprs/DerefExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    DerefExprSema::DerefExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto DerefExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("DerefExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
        });
    }

    auto DerefExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const DerefExprSema>>
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
            std::make_shared<const DerefExprSema>(
                GetSrcLocation(),
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }
    auto DerefExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto DerefExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const DerefExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const DerefExprSema>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto DerefExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto DerefExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr);
    }

    auto DerefExprSema::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropInfo> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );
            
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        ACE_ASSERT(typeSymbol->IsRef());

        auto* const type = llvm::PointerType::get(
            emitter.GetType(typeSymbol), 
            0
        );

        auto* const loadInst = emitter.GetBlock().Builder.CreateLoad(
            type,
            exprEmitResult.Value
        );
        tmps.emplace_back(loadInst, typeSymbol->GetWithoutRef());

        return { loadInst, tmps };
    }

    auto DerefExprSema::GetTypeInfo() const -> TypeInfo
    {
        const auto typeInfo = m_Expr->GetTypeInfo();

        auto* const typeSymbol = typeInfo.Symbol;
        ACE_ASSERT(typeSymbol->IsRef());

        return { typeSymbol->GetWithoutRef(), typeInfo.ValueKind };
    }

    auto DerefExprSema::GetExpr() const -> std::shared_ptr<const IExprSema>
    {
        return m_Expr;
    }
}
