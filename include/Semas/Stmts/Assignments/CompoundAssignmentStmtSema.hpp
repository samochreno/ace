#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class CompoundAssignmentStmtSema :
        public std::enable_shared_from_this<CompoundAssignmentStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<CompoundAssignmentStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<GroupStmtSema>
    {
    public:
        CompoundAssignmentStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& lhsExpr,
            const std::shared_ptr<const IExprSema>& rhsExpr,
            FunctionSymbol* const opSymbol
        );
        virtual ~CompoundAssignmentStmtSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const CompoundAssignmentStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const GroupStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSema> m_LHSExpr{};
        std::shared_ptr<const IExprSema> m_RHSExpr{};
        FunctionSymbol* m_OpSymbol{};
    };
}
