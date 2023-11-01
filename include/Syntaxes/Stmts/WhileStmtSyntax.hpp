#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"
#include "Semas/Stmts/WhileStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"

namespace Ace
{
    class WhileStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<WhileStmtSema>
    {
    public:
        WhileStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<const IExprSyntax>& condition,
            const std::shared_ptr<const BlockStmtSyntax>& block
        );
        virtual ~WhileStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const WhileStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<const IExprSyntax> m_Condition{};
        std::shared_ptr<const BlockStmtSyntax> m_Block{};
    };
}
