#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/CopyStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class CopyStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<CopyStmtSema>
    {
    public:
        CopyStmtSyntax(
            const SrcLocation& srcLocation,
            const TypeName& typeName, 
            const std::shared_ptr<const IExprSyntax>& srcExpr,
            const std::shared_ptr<const IExprSyntax>& dstExpr
        );
        virtual ~CopyStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const CopyStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        TypeName m_TypeName{};
        std::shared_ptr<const IExprSyntax> m_SrcExpr{};
        std::shared_ptr<const IExprSyntax> m_DstExpr{};
    };
}
