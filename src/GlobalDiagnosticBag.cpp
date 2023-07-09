#include "GlobalDiagnosticBag.hpp"

#include <memory>
#include <vector>

#include "DiagnosticBag.hpp"
#include "SourceBuffer.hpp"
#include "Log.hpp"

namespace Ace
{
    auto LogDiagnostic(
        const std::shared_ptr<const Diagnostic>& t_diagnostic
    ) -> void
    {
        switch (t_diagnostic->Severity)
        {
            case DiagnosticSeverity::Info:
            {
                Log << "info";
                break;
            }

            case DiagnosticSeverity::Warning:
            {
                Log << termcolor::bright_yellow << "warning";
            }

            case DiagnosticSeverity::Error:
            {
                Log << termcolor::bright_red << "error";
            }
        }

        Log << termcolor::reset << ": ";
        Log << t_diagnostic->Message << "\n";
        
        if (t_diagnostic->OptSourceLocation.has_value())
        {
            Log << termcolor::bright_blue << " --> ";
            Log << termcolor::reset;
            Log << t_diagnostic->OptSourceLocation.value().Buffer->FormatLocation(
                t_diagnostic->OptSourceLocation.value()
            );
            Log << "\n";
        }
    }

    auto LogGlobalDiagnostics(
        const DiagnosticBag& t_diagnosticBag,
        const size_t t_lastLogSize
    ) -> void
    {
        std::for_each(
            begin(t_diagnosticBag.GetDiagnostics()) + t_lastLogSize,
            end  (t_diagnosticBag.GetDiagnostics()),
            [&](const std::shared_ptr<const Diagnostic>& t_diagnostic)
            {
                if (t_diagnostic != *begin(t_diagnosticBag.GetDiagnostics()))
                {
                    Log << "\n";
                }

                LogDiagnostic(t_diagnostic);
            }
        );
    }
}
