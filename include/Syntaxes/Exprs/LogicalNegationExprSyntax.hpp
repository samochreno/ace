#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/LogicalNegationExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class LogicalNegationExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<LogicalNegationExprSema>
    {
    public:
        LogicalNegationExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSyntax>& expr
        );
        virtual ~LogicalNegationExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope>;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const LogicalNegationExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSyntax> m_Expr{};
    };
}
