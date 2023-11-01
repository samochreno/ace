#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ConstraintSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        ConstraintSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const SymbolName& typeName,
            const std::vector<SymbolName>& traitNames
        );
        virtual ~ConstraintSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_TypeName{};
        std::vector<SymbolName> m_TraitNames{};
    };
}
