#include "Syntaxes/ConstraintSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/ConstraintSymbol.hpp"

namespace Ace
{
    ConstraintSyntax::ConstraintSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& typeName,
        const std::vector<SymbolName>& traitNames
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_TypeName{ typeName },
        m_TraitNames{ traitNames }
    {
    }

    auto ConstraintSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ConstraintSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ConstraintSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto ConstraintSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto ConstraintSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::AfterType;
    }

    auto ConstraintSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            GetScope()->ResolveStaticSymbol<ITypeSymbol>(m_TypeName)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        std::vector<TraitTypeSymbol*> traitSymbols{};
        std::for_each(begin(m_TraitNames), end(m_TraitNames),
        [&](const SymbolName& traitName)
        {
            const auto optTraitSymbol = diagnostics.Collect(
                GetScope()->ResolveStaticSymbol<TraitTypeSymbol>(traitName)
            );
            if (optTraitSymbol.has_value())
            {
                traitSymbols.push_back(optTraitSymbol.value());
            }
        });

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<ConstraintSymbol>(
                GetSrcLocation(),
                GetSymbolScope(),
                typeSymbol,
                traitSymbols
            ),
            std::move(diagnostics),
        };
    }
}
