#include "Syntaxes/ImplSelfSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/Types/Aliases/ImplSelfAliasTypeSymbol.hpp"

namespace Ace
{
    ImplSelfSyntax::ImplSelfSyntax(
        const std::shared_ptr<Scope>& scope,
        const SymbolName& name
    ) : m_SrcLocation{ name.CreateSrcLocation() },
        m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto ImplSelfSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ImplSelfSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplSelfSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto ImplSelfSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto ImplSelfSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::TypeAlias;
    }

    auto ImplSelfSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optTypeSymbol = diagnostics.Collect(
            GetScope()->ResolveStaticSymbol<ISizedTypeSymbol>(m_Name)
        );
        auto* const typeSymbol = optTypeSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetSizedType()
        );

        return Diagnosed
        {
            std::make_unique<ImplSelfAliasTypeSymbol>(
                GetSrcLocation(),
                GetSymbolScope(),
                typeSymbol
            ),
            std::move(diagnostics),
        };
    }
}
