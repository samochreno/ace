#include "Expected.hpp"

#include <memory>

#include "DiagnosticBase.hpp"

namespace Ace
{
    auto CreateEmptyError() -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            std::nullopt,
            "<Empty error>"
        );

        return group;
    }
}
