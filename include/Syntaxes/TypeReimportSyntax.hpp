#pragma once 

#include <memory>

#include "Syntaxes/Syntax.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    class TypeReimportSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        TypeReimportSyntax(
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<Scope>& reimportScope,
            const Ident& name
        );
        virtual ~TypeReimportSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_ReimportScope{};
        Ident m_Name{};
    };
}
