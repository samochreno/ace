#include "Semas/Stmts/DropStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/EmittableTypeSymbol.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprDropInfo.hpp"

namespace Ace
{
    DropStmtSema::DropStmtSema(
        const SrcLocation& srcLocation,
        ISizedTypeSymbol* const typeSymbol,
        const std::shared_ptr<const IExprSema>& expr
    ) : m_SrcLocation{ srcLocation },
        m_TypeSymbol{ typeSymbol },
        m_Expr{ expr }
    {
    }

    auto DropStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("DropStmtSema", [&]()
        {
            logger.Log("m_TypeSymbol", m_TypeSymbol);
            logger.Log("m_Expr", m_Expr);
        });
    }

    auto DropStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DropStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DropStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const DropStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        auto* const ptrTypeSymbol =
            GetCompilation()->GetNatives().Ptr.GetSymbol();

        if (checkedExpr->GetTypeInfo().Symbol != ptrTypeSymbol)
        {
            diagnostics.Add(CreateExpectedPtrError(
                checkedExpr->GetSrcLocation()
            ));
        }

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const DropStmtSema>(
                m_SrcLocation,
                m_TypeSymbol,
                checkedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto DropStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto DropStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const DropStmtSema>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr(context);

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const DropStmtSema>(
            m_SrcLocation,
            m_TypeSymbol,
            loweredExpr
        );
    }

    auto DropStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto DropStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}
            .Collect(m_TypeSymbol)
            .Collect(m_Expr);
    }

    auto DropStmtSema::Emit(Emitter& emitter) const -> void
    {
        std::vector<ExprDropInfo> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );

        auto* const instantiatedTypeSymbol =
            emitter.CreateInstantiated<IConcreteTypeSymbol>(m_TypeSymbol);

        auto* const glueSymbol = instantiatedTypeSymbol->GetDropGlue().value();

        emitter.GetBlock().Builder.CreateCall(
            emitter.GetFunction(glueSymbol),
            { exprEmitResult.Value }
        );
    }

    auto DropStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return {};
    }
}
