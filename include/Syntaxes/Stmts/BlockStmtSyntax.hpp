#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Semas/Stmts/BlockStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class BlockStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<BlockStmtSema>
    {
    public:
        BlockStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& bodyScope,
            const std::vector<std::shared_ptr<const IStmtSyntax>>& stmts
        );
        virtual ~BlockStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const BlockStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_BodyScope{};
        std::vector<std::shared_ptr<const IStmtSyntax>> m_Stmts{};
    };
}
