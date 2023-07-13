#include "Expected.hpp"

#include <memory>

#include "DiagnosticBase.hpp"

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
