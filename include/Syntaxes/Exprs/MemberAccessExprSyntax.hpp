#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/VarRefs/FieldVarRefExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class MemberAccessExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<FieldVarRefExprSema>
    {
    public:
        MemberAccessExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<const IExprSyntax>& expr,
            const SymbolNameSection& name
        );
        virtual ~MemberAccessExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope>;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const FieldVarRefExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

        auto GetExpr() const -> const IExprSyntax*;
        auto GetName() const -> const SymbolNameSection&;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<const IExprSyntax> m_Expr{};
        SymbolNameSection m_Name{};
    };
}
