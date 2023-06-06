#pragma once

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
    };
}
