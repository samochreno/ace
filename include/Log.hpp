#pragma once

#include <optional>
#include <string_view>

#include "DiagnosticsBase.hpp"
#include "SourceBuffer.hpp"

namespace Ace
{
    auto Log(
        const DiagnosticSeverity t_severity,
        const std::string_view t_message,
        const std::optional<SourceLocation>& t_optSourceLocation = std::nullopt
    )  -> void;
    auto LogFlush() -> void;
}
