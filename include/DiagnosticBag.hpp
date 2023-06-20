#pragma once

#include <memory>
#include <vector>

#include "Asserts.hpp"
#include "Expected.hpp"
#include "DiagnosticsBase.hpp"

namespace Ace
{
    class DiagnosticBag
    {
    public:
        DiagnosticBag() = default;
        ~DiagnosticBag() = default;

        auto Add(const std::shared_ptr<const IDiagnostic>& t_diagnostic) -> void
        {
            m_Diagnostics.push_back(t_diagnostic);
            AddSeverity(t_diagnostic->GetSeverity());
        }
        template<typename TDiagnosticNew, typename... A>
        auto Add(A&&... args) -> void
        {
            Add(std::make_shared<TDiagnosticNew>(
                std::forward<A>(args)...
            ));
        }
        auto Add(
            const DiagnosticBag& t_diagnosticBag
        ) -> void
        {
            std::for_each(
                begin(t_diagnosticBag.GetDiagnostics()),
                end  (t_diagnosticBag.GetDiagnostics()),
                [&](const std::shared_ptr<const IDiagnostic>& t_diagnostic)
                {
                    Add(t_diagnostic);
                }
            );
        }
        auto Add(
            const Expected<void>& t_expected
        ) -> void
        {
            if (t_expected)
                return;

            Add(t_expected.GetError());
        }

        auto GetDiagnostics() const -> const std::vector<std::shared_ptr<const IDiagnostic>>& { return m_Diagnostics; }
        auto GetSeverity() const -> DiagnosticSeverity { return m_Severity; }

    private:
        auto AddSeverity(const DiagnosticSeverity& t_severity) -> void
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

                default:
                {
                    ACE_UNREACHABLE();
                }
            }
        }

        std::vector<std::shared_ptr<const IDiagnostic>> m_Diagnostics{};
        DiagnosticSeverity m_Severity = DiagnosticSeverity::Info;
    };
}
