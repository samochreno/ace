#pragma once

#include <vector>
#include <string>
#include <optional>

#include "SrcLocation.hpp"

namespace Ace
{
    enum class DiagnosticSeverity
    {
        Info,
        Note,
        Warning,
        Error,
    };

    struct Diagnostic
    {
        DiagnosticSeverity Severity{};
        std::optional<SrcLocation> OptSrcLocation{};
        std::string Message{};
    };

    struct DiagnosticGroup
    {
        std::vector<Diagnostic> Diagnostics{};
    };
}
