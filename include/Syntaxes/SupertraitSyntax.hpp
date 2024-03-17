#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Name.hpp"
#include "Ident.hpp"
#include "Scope.hpp"
#include "SrcLocation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class SupertraitSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        SupertraitSyntax(
            const SymbolName& name,
            const Ident& parentName,
            const std::shared_ptr<Scope>& scope
        );
        virtual ~SupertraitSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        SymbolName m_Name{};
        Ident m_ParentName{};
        std::shared_ptr<Scope> m_Scope{};
    };
}

