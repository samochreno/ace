#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class ExitStmtSema : 
        public std::enable_shared_from_this<ExitStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<ExitStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<ExitStmtSema>
    {
    public:
        ExitStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope
        );
        virtual ~ExitStmtSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const ExitStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const ExitStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
    };
}
