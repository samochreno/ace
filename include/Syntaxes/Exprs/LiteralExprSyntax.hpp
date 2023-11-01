#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/LiteralExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class LiteralExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<LiteralExprSema>
    {
    public:
        LiteralExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const LiteralKind kind,
            const std::string& string
        );
        virtual ~LiteralExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const LiteralExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
