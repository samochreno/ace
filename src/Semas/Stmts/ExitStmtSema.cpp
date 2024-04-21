#include "Semas/Stmts/ExitStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "SemaLogger.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    ExitStmtSema::ExitStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope }
    {
    }

    auto ExitStmtSema::Log(SemaLogger& logger) const -> void
    {
        logger.Log("ExitStmtSema", [&]() {});
    }

    auto ExitStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ExitStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ExitStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ExitStmtSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto ExitStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto ExitStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ExitStmtSema>
    {
        return shared_from_this();
    }
    
    auto ExitStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto ExitStmtSema::CollectMonos() const -> MonoCollector
    {
        return {};
    }

    auto ExitStmtSema::Emit(Emitter& emitter) const -> void
    {
        const auto srcLocation = GetSrcLocation();
        const auto location = srcLocation.Buffer->FormatLocation(srcLocation);

        emitter.EmitPrintf(emitter.EmitString("Aborted at " + location));

        auto* const argValue = llvm::ConstantInt::get(
            emitter.GetC().GetTypes().GetInt(),
            -1,
            true
        );

        emitter.GetBlock().Builder.CreateCall(
            emitter.GetC().GetFunctions().GetExit(),
            { argValue }
        );

        emitter.GetBlock().Builder.CreateUnreachable();
    }

    auto ExitStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return std::vector{ ControlFlowNode{ ControlFlowKind::Exit } };
    }
}
