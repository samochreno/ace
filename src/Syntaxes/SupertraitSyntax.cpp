#include "Syntaxes/SupertraitSyntax.hpp"

#include <memory>
#include <vector>

#include "Name.hpp"
#include "Ident.hpp"
#include "Scope.hpp"
#include "SrcLocation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "Symbols/SupertraitSymbol.hpp"

namespace Ace
{
    SupertraitSyntax::SupertraitSyntax(
        const SymbolName& name,
        const Ident& parentName,
        const std::shared_ptr<Scope>& scope
    ) : m_SrcLocation{ name.CreateSrcLocation() },
        m_Name{ name },
        m_ParentName{ parentName },
        m_Scope{ scope }
    {
    }

    auto SupertraitSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SupertraitSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SupertraitSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto SupertraitSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }
    
    auto SupertraitSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    static auto IsCycle(
        TraitTypeSymbol* traitSymbol,
        ITypeSymbol* supertraitSymbol
    ) -> bool
    {
        traitSymbol = dynamic_cast<TraitTypeSymbol*>(
            traitSymbol->GetUnaliased()->GetRoot()
        );
        ACE_ASSERT(traitSymbol);

        supertraitSymbol = dynamic_cast<ITypeSymbol*>(
            supertraitSymbol->GetUnaliased()->GetRoot()
        );
        ACE_ASSERT(supertraitSymbol);

        if (traitSymbol == supertraitSymbol)
        {
            return true;
        }

        const auto typeArgs = supertraitSymbol->GetTypeArgs();
        const auto cyclicTypeArgIt = std::find_if(
            begin(typeArgs),
            end  (typeArgs),
            [&](ITypeSymbol* const typeArg)
            {
                auto* const typeArgType = dynamic_cast<ITypeSymbol*>(
                    typeArg->GetUnaliased()->GetRoot()
                );
                ACE_ASSERT(typeArgType);
                return IsCycle(traitSymbol, typeArgType);
            }
        );
        if (cyclicTypeArgIt != end(typeArgs))
        {
            return true;
        }

        return false;
    }

    auto SupertraitSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const parentTraitSymbol = DiagnosticBag::CreateNoError().Collect(
            GetScope()->ResolveStaticSymbol<TraitTypeSymbol>(m_ParentName)
        ).value();

        const auto optTraitSymbol = diagnostics.Collect(
            GetScope()->ResolveStaticSymbol<TraitTypeSymbol>(m_Name)
        );
        auto* traitSymbol = optTraitSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetTrait()
        );

        if (IsCycle(parentTraitSymbol, traitSymbol))
        {
            diagnostics.Add(CreateSupertraitCausesCycleError(GetSrcLocation()));
            traitSymbol = GetCompilation()->GetErrorSymbols().GetTrait();
        }

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<SupertraitSymbol>(
                GetSrcLocation(),
                GetSymbolScope(),
                traitSymbol
            ),
            std::move(diagnostics),
        };
    }
}
