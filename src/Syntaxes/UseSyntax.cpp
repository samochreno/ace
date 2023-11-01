#include "Syntaxes/UseSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/UseSymbol.hpp"
#include "Diagnostics/SymbolTypeDiagnostics.hpp"

namespace Ace
{
    UseSyntax::UseSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& rootTraitName
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_RootTraitName{ rootTraitName }
    {
    }

    auto UseSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UseSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto UseSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto UseSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto UseSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    static auto ResolveRootTraitSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& rootTraitName
    ) -> Expected<TraitTypeSymbol*>
    {
        auto typeDiagnostics = DiagnosticBag::Create();
        const auto optSymbol = typeDiagnostics.Collect(
            scope->ResolveStaticSymbol<TraitTypeSymbol>(rootTraitName)
        );
        if (optSymbol.has_value())
        {
            auto* const symbol = optSymbol.value();
            ACE_ASSERT(symbol == symbol->GetRoot());
            return Expected{ symbol, std::move(typeDiagnostics) };
        }

        auto diagnostics = DiagnosticBag::Create();

        const auto optRootSymbol = diagnostics.Collect(
            scope->ResolveRoot<TraitTypeSymbol>(rootTraitName)
        );
        if (!optRootSymbol.has_value())
        {
            return std::move(typeDiagnostics);
        }

        return Expected
        {
            optRootSymbol.value(),
            std::move(diagnostics),
        };
    }

    auto UseSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optRootTraitSymbol = diagnostics.Collect(
            ResolveRootTraitSymbol(
                GetSrcLocation(),
                GetScope(),
                m_RootTraitName
            )
        );
        auto* const rootTraitSymbol = optRootTraitSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetTrait()
        );

        return Diagnosed
        {
            std::make_unique<UseSymbol>(
                GetSrcLocation(),
                GetSymbolScope(),
                rootTraitSymbol
            ),
            std::move(diagnostics),
        };
    }
}
