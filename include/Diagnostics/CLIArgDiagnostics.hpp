#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    inline auto CreateMissingCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing option name"
        );
    }

    inline auto CreateUnknownCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unknown option name"
        );
    }

    inline auto CreateMissingCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing option argument"
        );
    }

    inline auto CreateUnexpectedCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unexpected option argument"
        );
    }
}
