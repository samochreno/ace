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
    class CopyStmtSema : 
        public std::enable_shared_from_this<CopyStmtSema>,
        public virtual IStmtSema,
        public virtual ITypeCheckableSema<CopyStmtSema, StmtTypeCheckingContext>,
        public virtual ILowerableSema<CopyStmtSema>
    {
    public:
        CopyStmtSema(
            const SrcLocation& srcLocation,
            ISizedTypeSymbol* const typeSymbol,
            const std::shared_ptr<const IExprSema>& srcExpr,
            const std::shared_ptr<const IExprSema>& dstExpr
        );
        virtual ~CopyStmtSema() = default;

        auto Log(SemaLogger& logger) const -> void final;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CreateTypeChecked(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const CopyStmtSema>> final;
        auto CreateTypeCheckedStmt(
            const StmtTypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const CopyStmtSema> final;
        auto CreateLoweredStmt(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IStmtSema> final;
        auto CollectMonos() const -> MonoCollector final;
        auto Emit(Emitter& emitter) const -> void final;

        auto CreateControlFlowNodes() const -> std::vector<ControlFlowNode> final;

    private:
        SrcLocation m_SrcLocation{};
        ISizedTypeSymbol* m_TypeSymbol{};
        std::shared_ptr<const IExprSema> m_SrcExpr{};
        std::shared_ptr<const IExprSema> m_DstExpr{};
    };
}
