#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "SrcLocation.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class CastExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<IExprSema>
    {
    public:
        CastExprSyntax(
            const SrcLocation& srcLocation,
            const TypeName& typeName,
            const std::shared_ptr<const IExprSyntax>& expr
        );
        virtual ~CastExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        TypeName m_TypeName{};
        std::shared_ptr<const IExprSyntax> m_Expr{};
    };
}
