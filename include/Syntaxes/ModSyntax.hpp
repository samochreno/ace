#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/Impls/InherentImplSyntax.hpp"
#include "Syntaxes/Impls/TraitImplSyntax.hpp"
#include "Syntaxes/FunctionSyntax.hpp"
#include "Syntaxes/Vars/GlobalVarSyntax.hpp"
#include "Syntaxes/UseSyntax.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class ModSyntax :
        public virtual ISyntax,
        public virtual IPartialDeclSyntax
    {
    public:
        ModSyntax(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const std::shared_ptr<Scope>& bodyScope,
            const std::vector<Ident>& name,
            const AccessModifier accessModifier,
            const std::vector<std::shared_ptr<const ModSyntax>>& mods,
            const std::vector<std::shared_ptr<const ISyntax>>& types,
            const std::vector<std::shared_ptr<const InherentImplSyntax>>& inherentImpls,
            const std::vector<std::shared_ptr<const TraitImplSyntax>>& traitImpls,
            const std::vector<std::shared_ptr<const FunctionSyntax>>& functions,
            const std::vector<std::shared_ptr<const GlobalVarSyntax>>& globalVars,
            const std::vector<std::shared_ptr<const UseSyntax>>& uses
        );
        virtual ~ModSyntax() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const ISyntax*> final;

        auto GetSymbolScope() const -> std::shared_ptr<Scope> final;
        auto GetDeclOrder() const -> DeclOrder final;
        auto GetDeclSuborder() const -> size_t final;
        auto CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>> final;
        auto ContinueCreatingSymbol(
            ISymbol* const symbol
        ) const -> Diagnosed<void> final;
        auto GetName() const -> const Ident& final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_BodyScope{};
        std::vector<Ident> m_Name{};
        AccessModifier m_AccessModifier{};
        std::vector<std::shared_ptr<const ModSyntax>> m_Mods{};
        std::vector<std::shared_ptr<const ISyntax>> m_Types{};
        std::vector<std::shared_ptr<const InherentImplSyntax>> m_InherentImpls{};
        std::vector<std::shared_ptr<const TraitImplSyntax>> m_TraitImpls{};
        std::vector<std::shared_ptr<const FunctionSyntax>> m_Functions{};
        std::vector<std::shared_ptr<const GlobalVarSyntax>> m_GlobalVars{};
        std::vector<std::shared_ptr<const UseSyntax>> m_Uses{};
    };
}
