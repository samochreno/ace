#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "Semas/Stmts/GroupStmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class IfStmtSema :
        public std::enable_shared_from_this<IfStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<IfStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<GroupStmtSema>
    {
    public:
        IfStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::vector<std::shared_ptr<const IExprSema>>& conditions,
            const std::vector<std::shared_ptr<const BlockStmtSema>>& blocks
        );
        virtual ~IfStmtSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IfStmtSema>> final;
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
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const IExprSema>> m_Conditions{};
        std::vector<std::shared_ptr<const BlockStmtSema>> m_Blocks{};
    };
}
