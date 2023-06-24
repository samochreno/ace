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
        ) -> DiagnosticBag&;
        
        template<typename TDiagnosticNew, typename... A>
        auto Add(A&&... args) -> DiagnosticBag&
        {
            Add(std::make_shared<TDiagnosticNew>(
                std::forward<A>(args)...
            ));

            return *this;
        }
        template<typename TDiagnosed, typename = std::enable_if_t<std::is_base_of_v<IDiagnosed, TDiagnosed>>>
        auto Add(const TDiagnosed& t_diagnosed) -> DiagnosticBag&
        {
            return Add(t_diagnosed.GetDiagnosticBag());
        }
        auto Add(const DiagnosticBag& t_diagnosticBag) -> DiagnosticBag&;

        auto IsEmpty() const -> bool;
        auto GetDiagnostics() const -> const std::vector<std::shared_ptr<const IDiagnostic>>&;
        auto GetSeverity() const -> DiagnosticSeverity;

    private:
        auto AddSeverity(const DiagnosticSeverity& t_severity) -> void;

        std::vector<std::shared_ptr<const IDiagnostic>> m_Diagnostics{};
        DiagnosticSeverity m_Severity = DiagnosticSeverity::Info;
    };
}
