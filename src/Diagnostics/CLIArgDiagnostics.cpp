#include "Diagnostics/CLIArgDiagnostics.hpp"

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateMissingCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing option name"
        );

        return group;
    }

    auto CreateUnknownCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unknown option name"
        );

        return group;
    }

    auto CreateMissingCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing option argument"
        );

        return group;
    }

    auto CreateUnexpectedCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unexpected option argument"
        );

        return group;
    }
}
