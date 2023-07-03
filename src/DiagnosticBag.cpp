#include "DiagnosticBag.hpp"

#include <memory>

#include "DiagnosticsBase.hpp"

namespace Ace
{
    auto DiagnosticBag::Add(
        const std::shared_ptr<const Diagnostic>& t_diagnostic
    ) -> DiagnosticBag&
    {
        m_Diagnostics.push_back(t_diagnostic);
        AddSeverity(t_diagnostic->Severity);

        return *this;
    }

    auto DiagnosticBag::Add(
        const DiagnosticBag& t_diagnosticBag
    ) -> DiagnosticBag&
    {
        std::for_each(
            begin(t_diagnosticBag.GetDiagnostics()),
            end  (t_diagnosticBag.GetDiagnostics()),
            [&](const std::shared_ptr<const Diagnostic>& t_diagnostic)
            {
                Add(t_diagnostic);
            }
        );

        return *this;
    }

    auto DiagnosticBag::IsEmpty() const -> bool
    {
        return m_Diagnostics.empty();
    }

    auto DiagnosticBag::GetDiagnostics() const -> const std::vector<std::shared_ptr<const Diagnostic>>&
    {
        return m_Diagnostics;
    }

    auto DiagnosticBag::GetSeverity() const -> DiagnosticSeverity
    {
        return m_Severity;
    }

    auto DiagnosticBag::AddSeverity(
        const DiagnosticSeverity& t_severity
    ) -> void
    {
        switch (t_severity)
        {
            case DiagnosticSeverity::Info:
            {
                break;
            }

            case DiagnosticSeverity::Warning:
            {
                if (m_Severity == DiagnosticSeverity::Info)
                {
                    m_Severity = DiagnosticSeverity::Warning;
                }

                break;
            }

            case DiagnosticSeverity::Error:
            {
                m_Severity = DiagnosticSeverity::Error;
                break;
            }
        }
    }
}
