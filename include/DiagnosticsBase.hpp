#pragma once

#include <string>
#include <optional>

#include "SourceLocation.hpp"

namespace Ace
{
    enum class DiagnosticSeverity
    {
        None,
        Info,
        Warning,
        Error,
    };

    class IDiagnostic
    {
    public:
        virtual ~IDiagnostic() = default;

        virtual auto GetSeverity() const -> DiagnosticSeverity = 0;
        virtual auto GetSourceLocation() const -> std::optional<SourceLocation> = 0;
        virtual auto CreateMessage() const -> std::string = 0;
    };
}
