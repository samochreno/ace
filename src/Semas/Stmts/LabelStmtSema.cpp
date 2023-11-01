#include "Semas/Stmts/LabelStmtSema.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    LabelStmtSema::LabelStmtSema(
        const SrcLocation& srcLocation,
        LabelSymbol* const symbol
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol }
    {
    }

    auto LabelStmtSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LabelStmtSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto LabelStmtSema::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const LabelStmtSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto LabelStmtSema::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtSema>>
    {
        return CreateTypeChecked(context);
    }

    auto LabelStmtSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const LabelStmtSema>
    {
        return shared_from_this();
    }

    auto LabelStmtSema::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtSema>
    {
        return CreateLowered(context);
    }

    auto LabelStmtSema::CollectMonos() const -> MonoCollector
    {
        return {};
    }

    auto LabelStmtSema::Emit(Emitter& emitter) const -> void
    {
    }

    auto LabelStmtSema::CreateControlFlowNodes() const -> std::vector<ControlFlowNode>
    {
        return std::vector
        {
            ControlFlowNode{ ControlFlowKind::Label, m_Symbol }
        };
    }

    auto LabelStmtSema::GetSymbol() const -> LabelSymbol*
    {
        return m_Symbol;
    }
}
