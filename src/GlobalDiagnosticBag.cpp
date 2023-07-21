#include "GlobalDiagnosticBag.hpp"

#include <memory>
#include <vector>

#include "DiagnosticBag.hpp"
#include "SourceBuffer.hpp"
#include "Log.hpp"

namespace Ace
{
    auto LogDiagnostic(
        const std::shared_ptr<const Diagnostic>& diagnostic
    ) -> void
    {
        switch (diagnostic->Severity)
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
        Log << diagnostic->Message << "\n";
        
        if (diagnostic->OptSourceLocation.has_value())
        {
            Log << termcolor::bright_blue << " --> ";
            Log << termcolor::reset;
            Log << diagnostic->OptSourceLocation.value().Buffer->FormatLocation(
                diagnostic->OptSourceLocation.value()
            );
            Log << "\n";
        }
    }

    auto LogGlobalDiagnostics(
        const DiagnosticBag& diagnosticBag,
        const size_t lastLogSize
    ) -> void
    {
        std::for_each(
            begin(diagnosticBag.GetDiagnostics()) + lastLogSize,
            end  (diagnosticBag.GetDiagnostics()),
            [&](const std::shared_ptr<const Diagnostic>& diagnostic)
            {
                if (diagnostic != *begin(diagnosticBag.GetDiagnostics()))
                {
                    Log << "\n";
                }

                LogDiagnostic(diagnostic);
            }
        );
    }
}
