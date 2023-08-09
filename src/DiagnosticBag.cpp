#include "DiagnosticBag.hpp"

#include <memory>

#include "DiagnosticBase.hpp"
#include "DiagnosticLog.hpp"

namespace Ace
{
    auto DiagnosticBag::Create() -> DiagnosticBag
    {
        return {};
    }

    auto DiagnosticBag::CreateGlobal() -> DiagnosticBag
    {
        auto diagnosticBag = Create();

        diagnosticBag.m_OptHandler = [](const DiagnosticGroup& diagnosticGroup)
        {
            LogDiagnosticGroup(diagnosticGroup);
        };

        return diagnosticBag;
    }

    auto DiagnosticBag::Add(DiagnosticBag diagnosticBag) -> DiagnosticBag&
    {
        std::for_each(
            begin(diagnosticBag.m_DiagnosticGroups),
            end  (diagnosticBag.m_DiagnosticGroups),
            [&](DiagnosticGroup& diagnosticGroup)
            {
                Add(std::move(diagnosticGroup));
            }
        );

        return *this;
    }

    auto DiagnosticBag::Add(DiagnosticGroup diagnosticGroup) -> DiagnosticBag&
    {
        std::for_each(
            begin(diagnosticGroup.Diagnostics),
            end  (diagnosticGroup.Diagnostics),
            [&](const Diagnostic& diagnostic)
            {
                AddSeverity(diagnostic.Severity);
            }
        );

        m_DiagnosticGroups.push_back(std::move(diagnosticGroup));

        if (m_OptHandler.has_value())
        {
            m_OptHandler.value()(m_DiagnosticGroups.back());
        }

        return *this;
    }

    auto DiagnosticBag::IsEmpty() const -> bool
    {
        return m_DiagnosticGroups.empty();
    }

    auto DiagnosticBag::GetSeverity() const -> DiagnosticSeverity
    {
        return m_Severity;
    }

    auto DiagnosticBag::HasErrors() const -> bool
    {
        return m_Severity == DiagnosticSeverity::Error;
    }

    auto DiagnosticBag::AddSeverity(const DiagnosticSeverity severity) -> void
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
