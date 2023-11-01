 #pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/AndExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class AndExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<AndExprSema>
    {
    public:
        AndExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSyntax>& lhsExpr,
            const std::shared_ptr<const IExprSyntax>& rhsExpr
        );
        virtual ~AndExprSyntax() = default;
    
        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const AndExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSyntax> m_LHSExpr{};
        std::shared_ptr<const IExprSyntax> m_RHSExpr{};
    };
}
