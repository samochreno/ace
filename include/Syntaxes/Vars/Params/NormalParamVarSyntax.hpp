#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class NormalParamVarSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        NormalParamVarSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const Ident& name,
            const TypeName& typeName,
            const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
            const size_t index
        );
        virtual ~NormalParamVarSyntax() = default;

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
        std::vector<std::shared_ptr<const AttributeSyntax>> m_Attributes{};
        size_t m_Index{};
    };
}
