#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Stmts/ExpandableStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class GroupStmtSema : 
        public std::enable_shared_from_this<GroupStmtSema>,
        public virtual IStmtSema,
        public virtual IExpandableStmtSema,
        public virtual ITypeCheckableSema<GroupStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<GroupStmtSema>
    {
    public:
        GroupStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::vector<std::shared_ptr<const IStmtSema>>& stmts
        );
        virtual ~GroupStmtSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const GroupStmtSema>> final;
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
        
        auto CreatePartiallyExpanded() const -> std::vector<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const IStmtSema>> m_Stmts{};
    };
}
