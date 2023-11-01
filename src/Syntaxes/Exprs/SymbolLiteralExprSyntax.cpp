#include "Syntaxes/Exprs/SymbolLiteralExprSyntax.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Semas/Exprs/VarRefs/StaticVarRefExprSema.hpp"
#include "Symbols/Vars/VarSymbol.hpp"

namespace Ace
{
    SymbolLiteralExprSyntax::SymbolLiteralExprSyntax(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const SymbolName& name
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto SymbolLiteralExprSyntax::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SymbolLiteralExprSyntax::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto SymbolLiteralExprSyntax::CollectChildren() const -> std::vector<const ISyntax*>
    {
        return SyntaxChildCollector{}.Build();
    }

    auto SymbolLiteralExprSyntax::CreateSema() const -> Diagnosed<std::shared_ptr<const StaticVarRefExprSema>>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optVarSymbol = diagnostics.Collect(
            GetScope()->ResolveStaticSymbol<IVarSymbol>(m_Name)
        );
        auto* const varSymbol = optVarSymbol.value_or(
            GetCompilation()->GetErrorSymbols().GetVar()
        );

        return Diagnosed
        {
            std::make_shared<const StaticVarRefExprSema>(
                GetSrcLocation(),
                GetScope(),
                varSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto SymbolLiteralExprSyntax::CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateSema();
    }

    auto SymbolLiteralExprSyntax::GetName() const -> const SymbolName&
    {
        return m_Name;
    }
}
