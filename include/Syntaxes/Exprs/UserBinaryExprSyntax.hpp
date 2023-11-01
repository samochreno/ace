#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/UserBinaryExprSema.hpp"
#include "SrcLocation.hpp"
#include "Op.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class UserBinaryExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<UserBinaryExprSema>
    {
    public:
        UserBinaryExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSyntax>& lhsExpr,
            const std::shared_ptr<const IExprSyntax>& rhsExpr,
            const SrcLocation& opSrcLocation,
            const Op op
        );
        virtual ~UserBinaryExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const UserBinaryExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSyntax> m_LHSExpr{};
        std::shared_ptr<const IExprSyntax> m_RHSExpr{};
        SrcLocation m_OpSrcLocation{};
        Op m_Op{};
    };
}
