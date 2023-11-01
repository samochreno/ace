#include "Semas/Stmts/Jumps/NormalJumpStmtSema.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    NormalJumpStmtSema::NormalJumpStmtSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        LabelSymbol* const labelSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_LabelSymbol{ labelSymbol }
    {
    }

    auto NormalJumpStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalJumpStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalJumpStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const NormalJumpStmtSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto NormalJumpStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto NormalJumpStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const NormalJumpStmtSema>
    {
        return shared_from_this();
    }

    auto NormalJumpStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto NormalJumpStmtSema::CollectMonos() const -> MonoCollector
    {
        return {};
    }

    auto NormalJumpStmtSema::Emit(Emitter& emitter) const -> void
    {
        emitter.GetBlock().Builder.CreateBr(
            emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol)
        );
    }

    auto NormalJumpStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return std::vector
        {
            ControlFlowNode{ ControlFlowKind::Jump, m_LabelSymbol }
        };
    }
}
