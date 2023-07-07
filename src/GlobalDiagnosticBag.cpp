#include "GlobalDiagnosticBag.hpp"

#include <memory>
#include <vector>

#include "DiagnosticBag.hpp"
#include "Log.hpp"
#include "Core.hpp"

namespace Ace
{
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

                Core::LogDiagnostic(t_diagnostic);
            }
        );
    }
}
