#include "Semas/Stmts/AssertStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Semas/Exprs/LogicalNegationExprSema.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Semas/Stmts/IfStmtSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "Semas/Stmts/ExitStmtSema.hpp"
#include "SemaLogger.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    AssertStmtSema::AssertStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprSema>& condition
    ) : m_SrcLocation{ srcLocation },
        m_Condition{ condition }
    {
    }

    auto AssertStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("AssertStmtSema", [&]()
        {
            logger.Log("m_Condition", m_Condition);
        });
    }

    auto AssertStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AssertStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto AssertStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const AssertStmtSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const TypeInfo typeInfo
        {
            GetCompilation()->GetNatives().Bool.GetSymbol(),
            ValueKind::R,
        };
        const auto checkedCondition = diagnostics.Collect(
            CreateImplicitlyConvertedAndTypeChecked(m_Condition, typeInfo)
        );

        if (checkedCondition == m_Condition)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const AssertStmtSema>(
                GetSrcLocation(),
                checkedCondition
            ),
            std::move(diagnostics),
        };
    }

    auto AssertStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto AssertStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const GroupStmtSema>
    {
        const auto loweredCondition = m_Condition->CreateLoweredExpr({});

        const auto condition = std::make_shared<const LogicalNegationExprSema>(
            loweredCondition->GetSrcLocation(),
            loweredCondition
        );

        const auto blockScope = GetScope()->CreateChild();

        const auto exitStmt = std::make_shared<const ExitStmtSema>(
            GetSrcLocation(),
            blockScope
        );

        const auto blockStmt = std::make_shared<const BlockStmtSema>(
            GetSrcLocation(),
            blockScope,
            std::vector<std::shared_ptr<const IStmtSema>>{ exitStmt }
        );

        return std::make_shared<const IfStmtSema>(
            GetSrcLocation(),
            GetScope(),
            std::vector<std::shared_ptr<const IExprSema>>{ condition },
            std::vector<std::shared_ptr<const BlockStmtSema>>{ blockStmt }
        )->CreateLowered({});
    };

    auto AssertStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto AssertStmtSema::CollectMonos() const -> MonoCollector
    {
        ACE_UNREACHABLE();
    }

    auto AssertStmtSema::Emit(Emitter& emitter) const -> void
    {
        ACE_UNREACHABLE();
    }

    auto AssertStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        ACE_UNREACHABLE();
    }
}
