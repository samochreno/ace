#pragma once

#include <memory>
#include <vector>

#include "Semas/Stmts/StmtSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ControlFlow.hpp"

namespace Ace
{
    class DropStmtSema : 
        public std::enable_shared_from_this<DropStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<DropStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<DropStmtSema>
    {
    public:
        DropStmtSema(
            const SrcLocation& srcLocation,
            ISizedTypeSymbol* const typeSymbol,
            const std::shared_ptr<const IExprSema>& expr
        );
        virtual ~DropStmtSema() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const DropStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const DropStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

    private:
        SrcLocation m_SrcLocation{};
        ISizedTypeSymbol* m_TypeSymbol{};
        std::shared_ptr<const IExprSema> m_Expr{};
    };
}
