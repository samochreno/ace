#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/Assignments/SimpleAssignmentStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class SimpleAssignmentStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<SimpleAssignmentStmtSema>
    {
    public:
        SimpleAssignmentStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<const IExprSyntax>& lhsExpr,
            const std::shared_ptr<const IExprSyntax>& rhsExpr
        );
        virtual ~SimpleAssignmentStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const SimpleAssignmentStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;
        
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprSyntax> m_LHSExpr{};
        std::shared_ptr<const IExprSyntax> m_RHSExpr{};
    };
}
