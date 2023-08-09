#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateMissingCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnknownCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateMissingCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnexpectedCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;
}
