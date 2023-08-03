#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateMissingCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnknownCLIOptionNameError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateMissingCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedCLIOptionValueError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;
}
