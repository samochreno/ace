#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "Syntaxes/Stmts/StmtSyntax.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/Exprs/ExprSyntax.hpp"
#include "Semas/Stmts/VarStmtSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class VarStmtSyntax :
        public virtual IStmtSyntax,
        public virtual ISemaSyntax<VarStmtSema>,
        public virtual IDeclSyntax
    {
    public:
        VarStmtSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const Ident& name,
            const TypeName& typeName,
            const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
            const std::optional<std::shared_ptr<const IExprSyntax>>& optAssignedExpr
        );
        virtual ~VarStmtSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;
        auto CreateSema() const -> Diagnosed<std::shared_ptr<const VarStmtSema>> final;
        auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        TypeName m_TypeName{};
        std::vector<std::shared_ptr<const AttributeSyntax>> m_Attributes{};
        std::optional<std::shared_ptr<const IExprSyntax>> m_OptAssignedExpr{};
    };
}
