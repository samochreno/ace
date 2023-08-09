#include "Diagnostics/CFADiagnostics.hpp"

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateNotAllControlPathsReturnError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "not all control paths return a value"
        );

        return group;
    }
}
