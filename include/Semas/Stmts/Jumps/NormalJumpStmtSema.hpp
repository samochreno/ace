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
    class NormalJumpStmtSema :
        public std::enable_shared_from_this<NormalJumpStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<NormalJumpStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<NormalJumpStmtSema>
    {
    public:
        NormalJumpStmtSema(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            LabelSymbol* const labelSymbol
        );
        virtual ~NormalJumpStmtSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const NormalJumpStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const NormalJumpStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        LabelSymbol* m_LabelSymbol{};
    };
}
