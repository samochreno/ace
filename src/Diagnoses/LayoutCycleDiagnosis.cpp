#include "Diagnoses/LayoutCycleDiagnosis.hpp"

#include "Diagnostic.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/GenericSymbol.hpp"

namespace Ace
{
    static auto DiagnoseLayoutCycle(
        const ITypeSymbol* const typeSymbol
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (typeSymbol->IsError())
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        auto* const genericSymbol =
            dynamic_cast<const IGenericSymbol*>(typeSymbol);

        if (genericSymbol && genericSymbol->IsPlaceholder())
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        diagnostics.Collect(typeSymbol->DiagnoseCycle());
        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto DiagnoseLayoutCycles(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& scope = compilation->GetPackageBodyScope();

        const auto typeSymbols = scope->CollectSymbolsRecursive<ITypeSymbol>();
        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](ITypeSymbol* const typeSymbol)
        {
            diagnostics.Collect(DiagnoseLayoutCycle(typeSymbol));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
