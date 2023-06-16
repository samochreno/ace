#pragma once

#include <memory>
#include <vector>

#include "Asserts.hpp"
#include "DiagnosticsBase.hpp"

namespace Ace
{
    template<typename TDiagnostic>
    class DiagnosticBag
    {
    public:
        DiagnosticBag() = default;
        ~DiagnosticBag() = default;

        auto Add(const std::shared_ptr<const TDiagnostic>& t_diagnostic) -> void
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
        template<typename TDiagnosticNew>
        auto Add(
            const DiagnosticBag<TDiagnosticNew>& t_diagnosticBag
        ) -> void
        {
            std::for_each(
                begin(t_diagnosticBag.GetDiagnostics()),
                end  (t_diagnosticBag.GetDiagnostics()),
                [&](const std::shared_ptr<const TDiagnosticNew>& t_diagnostic)
                {
                    Add(t_diagnostic);
                }
            );
        }

        auto GetDiagnostics() const -> const std::vector<std::shared_ptr<const TDiagnostic>>& { return m_Diagnostics; }
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

        std::vector<std::shared_ptr<const TDiagnostic>> m_Diagnostics{};
        DiagnosticSeverity m_Severity = DiagnosticSeverity::Info;
    };
}
