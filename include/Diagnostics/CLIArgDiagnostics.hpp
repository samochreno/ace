#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    inline auto CreateMissingCLIOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "missing option name"
        );
    }

    inline auto CreateUnknownCLIOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unknown option name"
        );
    }

    inline auto CreateMissingCLIOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "missing option argument"
        );
    }

    inline auto CreateUnexpectedCLIOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unexpected option argument"
        );
    }
}
