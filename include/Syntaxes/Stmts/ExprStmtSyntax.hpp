#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/ExprStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ExprStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<ExprStmtSema>
    {
    public:
        ExprStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSyntax>& expr
        );
        virtual ~ExprStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const ExprStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSyntax> m_Expr{};
    };
}
