#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Semas/Stmts/ExitStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ExitStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<ExitStmtSema>
    {
    public:
        ExitStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope
        );
        virtual ~ExitStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const ExitStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
    };
}
