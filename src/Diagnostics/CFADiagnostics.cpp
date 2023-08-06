#include "Diagnostics/CFADiagnostics.hpp"

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateNotAllControlPathsReturnError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "not all control paths return a value"
        );

        return group;
    }
}
