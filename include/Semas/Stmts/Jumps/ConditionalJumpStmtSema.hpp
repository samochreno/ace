#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class ConditionalJumpStmtSema :
        public std::enable_shared_from_this<ConditionalJumpStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<ConditionalJumpStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<ConditionalJumpStmtSema>
    {
    public:
        ConditionalJumpStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& condition,
            LabelSymbol* const labelSymbol
        );
        virtual ~ConditionalJumpStmtSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ConditionalJumpStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ConditionalJumpStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSema> m_Condition{};
        LabelSymbol* m_LabelSymbol{};
    };
}
