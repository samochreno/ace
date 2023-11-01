#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    class SelfParamVarSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        SelfParamVarSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const TypeName& typeName
        );
        virtual ~SelfParamVarSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        TypeName m_TypeName{};
    };
}
