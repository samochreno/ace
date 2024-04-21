#include "Semas/Exprs/LogicalNegationExprSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    LogicalNegationExprSema::LogicalNegationExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto LogicalNegationExprSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("LogicalNegationExprSema", [&]()
        {
            logger.Log("m_Expr", m_Expr);
        });
    }

    auto LogicalNegationExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LogicalNegationExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const LogicalNegationExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
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
            std::make_shared<const LogicalNegationExprSema>(
                GetSrcLocation(),
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto LogicalNegationExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto LogicalNegationExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const LogicalNegationExprSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const LogicalNegationExprSema>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto LogicalNegationExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto LogicalNegationExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr);
    }

    auto LogicalNegationExprSema::Emit(
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

        auto* const boolType =
            emitter.GetType(GetCompilation()->GetNatives().Bool.GetSymbol());

        auto* const loadInst = emitter.GetBlock().Builder.CreateLoad(
            boolType,
            exprEmitResult.Value
        );

        auto* const negatedValue = emitter.GetBlock().Builder.CreateXor(
            loadInst,
            1
        );

        auto* const allocaInst = emitter.GetBlock().Builder.CreateAlloca(
            boolType
        );

        emitter.GetBlock().Builder.CreateStore(negatedValue, allocaInst);

        return { allocaInst, exprEmitResult.Tmps };
    }

    auto LogicalNegationExprSema::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R
        };
    }
}
