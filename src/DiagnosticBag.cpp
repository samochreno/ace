#include "DiagnosticBag.hpp"

#include <memory>

#include "DiagnosticBase.hpp"

namespace Ace
{
    auto DiagnosticBag::Add(
        const std::shared_ptr<const Diagnostic>& diagnostic
    ) -> DiagnosticBag&
    {
        m_Diagnostics.push_back(diagnostic);
        AddSeverity(diagnostic->Severity);

        return *this;
    }

    auto DiagnosticBag::Add(
        const DiagnosticBag& diagnosticBag
    ) -> DiagnosticBag&
    {
        std::for_each(
            begin(diagnosticBag.GetDiagnostics()),
            end  (diagnosticBag.GetDiagnostics()),
            [&](const std::shared_ptr<const Diagnostic>& diagnostic)
            {
                Add(diagnostic);
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

    auto DiagnosticBag::HasErrors() const -> bool
    {
        return m_Severity == DiagnosticSeverity::Error;
    }

    auto DiagnosticBag::AddSeverity(
        const DiagnosticSeverity& severity
    ) -> void
    {
        switch (severity)
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
