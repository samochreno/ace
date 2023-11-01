#include "Syntaxes/TypeReimportSyntax.hpp"

#include <memory>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Symbols/Types/Aliases/ReimportAliasTypeSymbol.hpp"

namespace Ace
{
    TypeReimportSyntax::TypeReimportSyntax(
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<Scope>& reimportScope,
        const Ident& name
    ) : m_Scope{ scope },
        m_ReimportScope{ reimportScope },
        m_Name{ name }
    {
    }

    auto TypeReimportSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_Name.SrcLocation;
    }

    auto TypeReimportSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto TypeReimportSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto TypeReimportSyntax::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TypeReimportSyntax::GetDeclOrder() const -> DeclOrder
    {
        return DeclOrder::TypeReimport;
    }

    auto TypeReimportSyntax::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optSymbol = diagnostics.Collect(
            m_ReimportScope->ResolveStaticSymbol<ITypeSymbol>(m_Name)
        );

        auto* const symbol = optSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetType()
        );

        return Diagnosed
        {
            std::make_unique<ReimportAliasTypeSymbol>(
                GetSymbolScope(),
                symbol
            ),
            std::move(diagnostics),
        };
    }
}
