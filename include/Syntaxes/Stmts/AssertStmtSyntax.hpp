#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/AssertStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class AssertStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<AssertStmtSema>
    {
    public:
        AssertStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<const IExprSyntax>& condition
        );
        virtual ~AssertStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const AssertStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprSyntax> m_Condition{};
    };
}
