#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "Asserts.hpp"
#include "DiagnosticsBase.hpp"

namespace Ace
{
    class DiagnosticBag
    {
    public:
        DiagnosticBag() = default;
        ~DiagnosticBag() = default;

        auto Add(
            const std::shared_ptr<const IDiagnostic>& t_diagnostic
        ) -> DiagnosticBag&
        {
            m_Diagnostics.push_back(t_diagnostic);
            AddSeverity(t_diagnostic->GetSeverity());

            return *this;
        }
        template<typename TDiagnosticNew, typename... A>
        auto Add(A&&... args) -> DiagnosticBag&
        {
            Add(std::make_shared<TDiagnosticNew>(
                std::forward<A>(args)...
            ));

            return *this;
        }
        auto Add(const DiagnosticBag& t_diagnosticBag) -> DiagnosticBag&
        {
            std::for_each(
                begin(t_diagnosticBag.GetDiagnostics()),
                end  (t_diagnosticBag.GetDiagnostics()),
                [&](const std::shared_ptr<const IDiagnostic>& t_diagnostic)
                {
                    Add(t_diagnostic);
                }
            );

            return *this;
        }
        template<typename TDiagnosed, typename = std::enable_if_t<std::is_base_of_v<IDiagnosed, TDiagnosed>>>
        auto Add(const TDiagnosed& t_diagnosed) -> DiagnosticBag&
        {
            return Add(t_diagnosed.GetDiagnosticBag());
        }

        auto IsEmpty() const -> bool { return m_Diagnostics.empty(); }
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
