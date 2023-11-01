#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "SrcLocation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class TraitSelfSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        TraitSelfSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope
        );
        virtual ~TraitSelfSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
    };
}

