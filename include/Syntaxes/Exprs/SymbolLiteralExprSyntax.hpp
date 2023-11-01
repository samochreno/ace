#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Exprs/VarRefs/StaticVarRefExprSema.hpp"
#include "Name.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class SymbolLiteralExprSyntax :
        public virtual IExprSyntax,
        public virtual ISemaSyntax<StaticVarRefExprSema>
    {
    public:
        SymbolLiteralExprSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const SymbolName& name
        );
        virtual ~SymbolLiteralExprSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const StaticVarRefExprSema>> final;
        auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> final;

        auto GetName() const -> const SymbolName&;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_Name;
    };
}
