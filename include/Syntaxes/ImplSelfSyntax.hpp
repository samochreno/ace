#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "SrcLocation.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ImplSelfSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        ImplSelfSyntax(
            const std::shared_ptr<Scope>& scope,
            const SymbolName& name
        );
        virtual ~ImplSelfSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_Name{};
    };
}

