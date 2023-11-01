#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"
#include "Semas/Stmts/IfStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class IfStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<IfStmtSema>
    {
    public:
        IfStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::vector<std::shared_ptr<const IExprSyntax>>& conditions,
            const std::vector<std::shared_ptr<const BlockStmtSyntax>>& blocks
        );
        virtual ~IfStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const IfStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::vector<std::shared_ptr<const IExprSyntax>> m_Conditions{};
        std::vector<std::shared_ptr<const BlockStmtSyntax>> m_Blocks{};
    };
}
