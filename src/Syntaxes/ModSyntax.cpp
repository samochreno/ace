#include "Syntaxes/ModSyntax.hpp"

#include <memory>
#include <vector>
#include <string>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Syntaxes/Impls/InherentImplSyntax.hpp"
#include "Syntaxes/Impls/TraitImplSyntax.hpp"
#include "Syntaxes/FunctionSyntax.hpp"
#include "Syntaxes/Vars/GlobalVarSyntax.hpp"
#include "Syntaxes/UseSyntax.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/ModSymbol.hpp"

namespace Ace
{
    ModSyntax::ModSyntax(
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
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_BodyScope{ bodyScope },
        m_Name{ name },
        m_AccessModifier{ accessModifier },
        m_Mods{ mods },
        m_Types{ types },
        m_InherentImpls{ inherentImpls },
        m_TraitImpls{ traitImpls },
        m_Functions{ functions },
        m_GlobalVars{ globalVars },
        m_Uses{ uses }
    {
    }

    auto ModSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ModSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ModSyntax::CollectChildren() const -> std::vector<const ISyntax*> 
    {
        return SyntaxChildCollector{}
            .Collect(m_Mods)
            .Collect(m_Types)
            .Collect(m_InherentImpls)
            .Collect(m_TraitImpls)
            .Collect(m_Functions)
            .Collect(m_GlobalVars)
            .Collect(m_Uses)
            .Build();
    }

    auto ModSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_BodyScope->GetParent().value();
    }

    auto ModSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::BeforeType;
    }

    auto ModSyntax::GetDeclSuborder() const -> size_t
    {
        return m_BodyScope->GetNestLevel();
    }

    auto ModSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<ModSymbol>(
                m_BodyScope,
                m_AccessModifier,
                GetName()
            ),
            DiagnosticBag::Create(),
        };
    }

    auto ModSyntax::ContinueCreatingSymbol(
        ISymbol* const symbol
    ) const -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const modSymbol = dynamic_cast<ModSymbol*>(symbol);
        ACE_ASSERT(modSymbol);

        if (modSymbol->GetAccessModifier() != m_AccessModifier)
        {
            const SrcLocation nameSrcLocation
            {
                m_Name.front().SrcLocation,
                m_Name.back().SrcLocation,
            };

            diagnostics.Add(CreateMismatchedAccessModifierError(
                modSymbol,
                nameSrcLocation,
                m_AccessModifier
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto ModSyntax::GetName() const -> const Ident&
    {
        return m_Name.back();
    }
}
