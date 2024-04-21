#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class RetStmtSema :
        public std::enable_shared_from_this<RetStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<RetStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<RetStmtSema>
    {
    public:
        RetStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::optional<std::shared_ptr<const IExprSema>>& optExpr
        );
        virtual ~RetStmtSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const RetStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const RetStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::optional<std::shared_ptr<const IExprSema>> m_OptExpr{};
    };
}
