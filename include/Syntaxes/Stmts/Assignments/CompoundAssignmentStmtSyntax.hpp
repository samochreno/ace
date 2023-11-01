#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/Assignments/CompoundAssignmentStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class CompoundAssignmentStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<CompoundAssignmentStmtSema>
    {
    public:
        CompoundAssignmentStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<const IExprSyntax>& lhsExpr,
            const std::shared_ptr<const IExprSyntax>& rhsExpr,
            const SrcLocation& opSrcLocation,
            const Op op
        );
        virtual ~CompoundAssignmentStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const CompoundAssignmentStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprSyntax> m_LHSExpr{};
        std::shared_ptr<const IExprSyntax> m_RHSExpr{};
        SrcLocation m_OpSrcLocation{};
        Op m_Op{};
    };
}
