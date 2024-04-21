#include "Semas/Stmts/RetStmtSema.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SemaLogger.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    RetStmtSema::RetStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::optional<std::shared_ptr<const IExprSema>>& optExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_OptExpr{ optExpr }
    {
    }

    auto RetStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("RetStmtSema", [&]()
        {
            logger.Log("m_OptExpr", m_OptExpr);
        });
    }

    auto RetStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto RetStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    static auto DiagnoseMissingOrUnexpectedExpr(
        const SrcLocation& srcLocation,
        ITypeSymbol* const functionTypeSymbol,
        const std::optional<std::shared_ptr<const IExprSema>>& optExpr
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const compilation = functionTypeSymbol->GetCompilation();

        const bool isFunctionTypeVoid =
            functionTypeSymbol == compilation->GetVoidTypeSymbol();

        if (!isFunctionTypeVoid && !optExpr.has_value())
        {
            diagnostics.Add(CreateMissingRetExprError(srcLocation));
        }

        if (isFunctionTypeVoid && optExpr.has_value())
        {
            diagnostics.Add(CreateExprRetFromVoidFunctionError(
                optExpr.value()->GetSrcLocation()
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseUnsizedExpr(
        const std::optional<std::shared_ptr<const IExprSema>>& optExpr
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!optExpr.has_value())
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        const auto& expr = optExpr.value();

        auto* const exprTypeSymbol =
            expr->GetTypeInfo().Symbol->GetUnaliasedType();

        if (!dynamic_cast<ISizedTypeSymbol*>(exprTypeSymbol))
        {
            diagnostics.Add(CreateUnsizedRetExprError(expr->GetSrcLocation()));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto RetStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const RetStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        diagnostics.Collect(DiagnoseMissingOrUnexpectedExpr(
            GetSrcLocation(),
            context.ParentFunctionTypeSymbol,
            m_OptExpr
        ));
        diagnostics.Collect(DiagnoseUnsizedExpr(m_OptExpr));

        std::optional<std::shared_ptr<const IExprSema>> checkedOptExpr{};
        if (m_OptExpr.has_value())
        {
            checkedOptExpr = diagnostics.Collect(
                CreateImplicitlyConvertedAndTypeChecked(
                    m_OptExpr.value(),
                    TypeInfo{ context.ParentFunctionTypeSymbol, ValueKind::R }
                )
            );
        }

        if (checkedOptExpr == m_OptExpr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const RetStmtSema>(
                GetSrcLocation(),
                GetScope(),
                checkedOptExpr
            ),
            std::move(diagnostics),
        };
    }

    auto RetStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto RetStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const RetStmtSema>
    {
        const auto loweredOptExpr = m_OptExpr.has_value() ?
            std::optional{ m_OptExpr.value()->CreateLoweredExpr({}) } :
            std::nullopt;

        if (loweredOptExpr == m_OptExpr)
        {
            return shared_from_this();
        }

        return std::make_shared<const RetStmtSema>(
            GetSrcLocation(),
            GetScope(),
            loweredOptExpr
        )->CreateLowered({});
    }

    auto RetStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto RetStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_OptExpr);
    }

    auto RetStmtSema::Emit(Emitter& emitter) const -> void
    {
        if (m_OptExpr.has_value())
        {
            const auto exprEmitResult = m_OptExpr.value()->Emit(emitter);
            
            auto* const typeSymbol = m_OptExpr.value()->GetTypeInfo().Symbol;
            auto* const type = emitter.GetType(typeSymbol);

            auto* const allocaInst =
                emitter.GetBlock().Builder.CreateAlloca(type);

            emitter.EmitCopy(allocaInst, exprEmitResult.Value, typeSymbol);

            emitter.EmitDropTmps(exprEmitResult.Tmps);
            emitter.EmitDropLocalVarsBeforeStmt(this);
            
            auto* const loadInst = emitter.GetBlock().Builder.CreateLoad(
                type,
                allocaInst
            );

            emitter.GetBlock().Builder.CreateRet(loadInst);
        }
        else
        {
            emitter.EmitDropLocalVarsBeforeStmt(this);

            emitter.GetBlock().Builder.CreateRetVoid();
        }
    }

    auto RetStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return std::vector{ ControlFlowNode{ ControlFlowKind::Ret } };
    }
}
