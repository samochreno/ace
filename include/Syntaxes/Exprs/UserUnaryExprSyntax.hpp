#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/UserUnaryExprSema.hpp"
#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class UserUnaryExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<UserUnaryExprSema>
    {
    public:
        UserUnaryExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSyntax>& expr,
            const SrcLocation& opSrcLocation,
            const Op op
        );
        virtual ~UserUnaryExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const UserUnaryExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
            
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSyntax> m_Expr{};
        SrcLocation m_OpSrcLocation{};
        Op m_Op{};
    };
}
