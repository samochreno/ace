#pragma once

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
        virtual auto GetMessage() const -> const char* = 0;
    };
}
