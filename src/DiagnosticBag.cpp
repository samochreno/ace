#include "DiagnosticBag.hpp"

#include <memory>

#include "DiagnosticBase.hpp"

namespace Ace
{
    auto DiagnosticBag::Add(
        const std::shared_ptr<const DiagnosticGroup>& diagnosticGroup
    ) -> DiagnosticBag&
    {
        m_DiagnosticGroups.push_back(diagnosticGroup);

        std::for_each(
            begin(diagnosticGroup->Diagnostics),
            end  (diagnosticGroup->Diagnostics),
            [&](const Diagnostic& diagnostic)
            {
                AddSeverity(diagnostic.Severity);
            }
        );

        return *this;
    }

    auto DiagnosticBag::Add(
        const DiagnosticBag& diagnostics
    ) -> DiagnosticBag&
    {
        std::for_each(
            begin(diagnostics.GetDiagnosticGroups()),
            end  (diagnostics.GetDiagnosticGroups()),
            [&](const std::shared_ptr<const DiagnosticGroup>& diagnosticGroup)
            {
                Add(diagnosticGroup);
            }
        );

        return *this;
    }

    auto DiagnosticBag::IsEmpty() const -> bool
    {
        return m_DiagnosticGroups.empty();
    }

    auto DiagnosticBag::GetDiagnosticGroups() const -> const std::vector<std::shared_ptr<const DiagnosticGroup>>&
    {
        return m_DiagnosticGroups;
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

            case DiagnosticSeverity::Note:
            {
                if (m_Severity == DiagnosticSeverity::Info)
                {
                    m_Severity = DiagnosticSeverity::Note;
                }

                break;
            }

            case DiagnosticSeverity::Warning:
            {
                if (
                    (m_Severity == DiagnosticSeverity::Info) ||
                    (m_Severity == DiagnosticSeverity::Note)
                    )
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
