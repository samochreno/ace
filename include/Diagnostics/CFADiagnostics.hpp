#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateNotAllControlPathsReturnError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;
}
