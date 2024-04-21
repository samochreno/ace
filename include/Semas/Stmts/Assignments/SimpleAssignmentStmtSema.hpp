#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class SimpleAssignmentStmtSema :
        public std::enable_shared_from_this<SimpleAssignmentStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<SimpleAssignmentStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<SimpleAssignmentStmtSema>
    {
    public:
        SimpleAssignmentStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSema>& lhsExpr,
            const std::shared_ptr<const IExprSema>& rhsExpr
        );
        virtual ~SimpleAssignmentStmtSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const SimpleAssignmentStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const SimpleAssignmentStmtSema> final;
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
    };
}
