#include "Expected.hpp"

#include <memory>

#include "DiagnosticsBase.hpp"

namespace Ace
{
    auto CreateEmptyError() -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "<Empty error>"
        );
    }
}
