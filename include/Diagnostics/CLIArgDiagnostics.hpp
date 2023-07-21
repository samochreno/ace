#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    inline auto CreateMissingCLIOptionNameError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "missing option name"
        );
    }

    inline auto CreateUnknownCLIOptionNameError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "unknown option name"
        );
    }

    inline auto CreateMissingCLIOptionValueError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "missing option argument"
        );
    }

    inline auto CreateUnexpectedCLIOptionValueError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "unexpected option argument"
        );
    }
}
