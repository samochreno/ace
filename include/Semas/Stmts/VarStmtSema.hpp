#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class VarStmtSema :
        public std::enable_shared_from_this<VarStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<VarStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<VarStmtSema>
    {
    public:
        VarStmtSema(
            const SrcLocation& srcLocation,
            LocalVarSymbol* const symbol,
            const std::optional<std::shared_ptr<const IExprSema>>& optAssignedExpr
        );
        virtual ~VarStmtSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const VarStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const VarStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

        auto GetSymbol() const -> LocalVarSymbol*;
        
    private:
        SrcLocation m_SrcLocation{};
        LocalVarSymbol* m_Symbol{};
        std::optional<std::shared_ptr<const IExprSema>> m_OptAssignedExpr{};
    };
}
