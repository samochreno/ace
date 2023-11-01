#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/Vars/FieldVarSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class StructSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        StructSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& bodyScope,
            const AccessModifier accessModifier,
            const Ident& name,
            const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
            const std::vector<std::shared_ptr<const FieldVarSyntax>>& fields,
            const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams
        );
        virtual ~StructSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;
        
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_BodyScope{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        std::vector<std::shared_ptr<const AttributeSyntax>> m_Attributes{};
        std::vector<std::shared_ptr<const FieldVarSyntax>> m_Fields{};
        std::vector<std::shared_ptr<const TypeParamSyntax>> m_TypeParams{};
    };
}
