#include "Semas/Stmts/VarStmtSema.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/VarRefs/StaticVarRefExprSema.hpp"
#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Stmts/Assignments/SimpleAssignmentStmtSema.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    VarStmtSema::VarStmtSema(
        const SrcLocation& srcLocation,
        LocalVarSymbol* const symbol,
        const std::optional<std::shared_ptr<const IExprSema>>& optAssignedExpr
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_OptAssignedExpr{ optAssignedExpr }
    {
    }

    auto VarStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("VarStmtSema", [&]()
        {
            logger.Log("m_Symbol", m_Symbol);
            logger.Log("m_OptAssignedExpr", m_OptAssignedExpr);
        });
    }

    auto VarStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto VarStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto VarStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const VarStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::optional<std::shared_ptr<const IExprSema>> checkedOptAssignedExpr{};
        if (m_OptAssignedExpr.has_value())
        {
            checkedOptAssignedExpr = diagnostics.Collect(
                CreateImplicitlyConvertedAndTypeChecked(
                    m_OptAssignedExpr.value(),
                    TypeInfo{ m_Symbol->GetType(), ValueKind::R }
                )
            );
        }

        if (checkedOptAssignedExpr == m_OptAssignedExpr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const VarStmtSema>(
                GetSrcLocation(),
                m_Symbol,
                checkedOptAssignedExpr
            ),
            std::move(diagnostics),
        };
    }

    auto VarStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto VarStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const VarStmtSema>
    {
        const auto loweredOptAssignedExpr = m_OptAssignedExpr.has_value() ?
            std::optional{ m_OptAssignedExpr.value()->CreateLoweredExpr({}) } :
            std::nullopt;

        if (loweredOptAssignedExpr == m_OptAssignedExpr)
        {
            return shared_from_this();
        }

        return std::make_shared<const VarStmtSema>(
            GetSrcLocation(),
            m_Symbol,
            loweredOptAssignedExpr
        )->CreateLowered({});
    }

    auto VarStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto VarStmtSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}
            .Collect(m_Symbol)
            .Collect(m_OptAssignedExpr);
    }

    auto VarStmtSema::Emit(Emitter& emitter) const -> void
    {
        if (!m_OptAssignedExpr.has_value())
        {
            return;
        }

        const auto varRefExpr = std::make_shared<const StaticVarRefExprSema>(
            m_Symbol->GetName().SrcLocation,
            GetScope(),
            m_Symbol
        );

        // Without type checking and implicit conversions,
        // refs can be initialized too
        const auto assignmentStmt = std::make_shared<const SimpleAssignmentStmtSema>(
            GetSrcLocation(),
            varRefExpr,
            m_OptAssignedExpr.value()
        );

        assignmentStmt->Emit(emitter);
    }

    auto VarStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return {};
    }

    auto VarStmtSema::GetSymbol() const -> LocalVarSymbol*
    {
        return m_Symbol;
    }
}
