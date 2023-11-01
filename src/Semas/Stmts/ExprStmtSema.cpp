#include "Semas/Stmts/ExprStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    ExprStmtSema::ExprStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_Expr{ expr }
    {
    }

    auto ExprStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExprStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto ExprStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ExprStmtSema>>
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
            std::make_shared<const ExprStmtSema>(
                GetSrcLocation(),
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto ExprStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto ExprStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ExprStmtSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const ExprStmtSema>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto ExprStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto ExprStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_Expr);
    }

    auto ExprStmtSema::Emit(Emitter& emitter) const -> void
    {
        const auto exprEmitResult = m_Expr->Emit(emitter);

        emitter.EmitDropTmps(exprEmitResult.Tmps);
    }

    auto ExprStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return {};
    }
}
