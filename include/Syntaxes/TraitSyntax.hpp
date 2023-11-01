#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/AttributeSyntax.hpp"
#include "Syntaxes/TraitSelfSyntax.hpp"
#include "Syntaxes/PrototypeSyntax.hpp"
#include "Syntaxes/TypeParamSyntax.hpp"
#include "Syntaxes/TypeReimportSyntax.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    class TraitSyntax :
        public virtual ISyntax,
        public virtual IDeclSyntax
    {
    public:
        TraitSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& bodyScope,
            const std::shared_ptr<Scope>& prototypeScope,
            const AccessModifier accessModifier,
            const Ident& name,
            const std::vector<std::shared_ptr<const AttributeSyntax>>& attributes,
            const std::shared_ptr<const TraitSelfSyntax>& self,
            const std::vector<std::shared_ptr<const PrototypeSyntax>>& prototypes,
            const std::vector<std::shared_ptr<const TypeParamSyntax>>& typeParams,
            const std::vector<std::shared_ptr<const TypeReimportSyntax>>& typeParamReimports 
        );
        virtual ~TraitSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;
        
    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_BodyScope{};
        std::shared_ptr<Scope> m_PrototypeScope{};
        AccessModifier m_AccessModifier{};
        Ident m_Name{};
        std::vector<std::shared_ptr<const AttributeSyntax>> m_Attributes{};
        std::shared_ptr<const TraitSelfSyntax> m_Self{};
        std::vector<std::shared_ptr<const PrototypeSyntax>> m_Prototypes{};
        std::vector<std::shared_ptr<const TypeParamSyntax>> m_TypeParams{};
        std::vector<std::shared_ptr<const TypeReimportSyntax>> m_TypeParamReimports{};
    };
}
