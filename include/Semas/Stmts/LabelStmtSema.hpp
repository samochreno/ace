#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class LabelStmtSema :
        public std::enable_shared_from_this<LabelStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<LabelStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<LabelStmtSema>
    {
    public:
        LabelStmtSema(
            const SrcLocation& srcLocation,
            LabelSymbol* const symbol
        );
        virtual ~LabelStmtSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const LabelStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const LabelStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

        auto GetSymbol() const -> LabelSymbol*;

    private:
        SrcLocation m_SrcLocation{};
        LabelSymbol* m_Symbol{};
    };
}
