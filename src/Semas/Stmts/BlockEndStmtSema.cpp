#include "Semas/Stmts/BlockEndStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "SemaLogger.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    BlockEndStmtSema::BlockEndStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& bodyScope
    ) : m_SrcLocation{ srcLocation },
        m_BodyScope{ bodyScope }
    {
    }

    auto BlockEndStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("BlockEndStmtSema", [&]() {});
    }

    auto BlockEndStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto BlockEndStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto BlockEndStmtSema::GetBodyScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope;
    }

    auto BlockEndStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const BlockEndStmtSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto BlockEndStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto BlockEndStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const BlockEndStmtSema>
    {
        return shared_from_this();
    }

    auto BlockEndStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto BlockEndStmtSema::CollectMonos() const -> MonoCollector
    {
        return {};
    }

    auto BlockEndStmtSema::Emit(Emitter& emitter) const -> void
    {
    }

    auto BlockEndStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return {};
    }
}
