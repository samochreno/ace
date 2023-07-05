#include "Log.hpp"

#include <iostream>
#include <string_view>
#include <termcolor/termcolor.hpp>
#include <optional>

#include "SourceBuffer.hpp"

#define ACE_LOG_COLOR_WARNING     243, 156,  18
#define ACE_LOG_COLOR_ERROR       192,  57,  43
#define ACE_LOG_COLOR_DEBUG       235, 108, 240

namespace Ace
{
    static auto LogSeverityColor(const DiagnosticSeverity t_severity) -> void
    {
        switch (t_severity)
        {
            case DiagnosticSeverity::Info:    std::cout << termcolor::reset;                        break;
            case DiagnosticSeverity::Warning: std::cout << termcolor::color<ACE_LOG_COLOR_WARNING>; break;
            case DiagnosticSeverity::Error:   std::cout << termcolor::color<ACE_LOG_COLOR_ERROR>;   break;
            case DiagnosticSeverity::Debug:   std::cout << termcolor::color<ACE_LOG_COLOR_DEBUG>;   break;
        }
    }

    static auto GetSeverityString(
        const DiagnosticSeverity t_severity
    ) -> const char*
    {
        switch (t_severity)
        {
            case DiagnosticSeverity::Info:    return "info";
            case DiagnosticSeverity::Warning: return "warning";
            case DiagnosticSeverity::Error:   return "error";
            case DiagnosticSeverity::Debug:   return "debug";
        }
    }

    auto Log(
        const DiagnosticSeverity t_severity,
        const std::string_view t_message,
        const std::optional<SourceLocation>& t_optSourceLocation
    ) -> void
    {
        if (t_optSourceLocation.has_value())
        {
            LogSeverityColor(DiagnosticSeverity::Info);
            std::cout << t_optSourceLocation.value().Buffer->FormatLocation(
                t_optSourceLocation.value()
            );
        }

        LogSeverityColor(t_severity);
        std::cout << GetSeverityString(t_severity);
        LogSeverityColor(DiagnosticSeverity::Info);
        std::cout << ": " << t_message << '\n';
    }

    auto LogFlush() -> void
    {
        std::cout << std::flush;
    }
}
