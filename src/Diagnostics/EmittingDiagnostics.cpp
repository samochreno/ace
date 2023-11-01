#include "Diagnostics/EmittingDiagnostics.hpp"

#include "Symbols/All.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    auto CreateMissingFunctionBlockError(
        FunctionSymbol* const function
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            function->GetName().SrcLocation,
            "missing function body"
        );

        return group;
    }
}
