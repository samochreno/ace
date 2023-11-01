#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class CallExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<IExprSema>
    {
    public:
        CallExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSyntax>& expr,
            const std::vector<std::shared_ptr<const IExprSyntax>>& args
        );
        virtual ~CallExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSyntax> m_Expr{};
        std::vector<std::shared_ptr<const IExprSyntax>> m_Args{};
    };
}
