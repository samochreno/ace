#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "Assert.hpp"
#include "DiagnosticBase.hpp"

namespace Ace
{
    class DiagnosticBag
    {
    public:
        DiagnosticBag() = default;
        ~DiagnosticBag() = default;

        auto Add(
            const std::shared_ptr<const Diagnostic>& diagnostic
        ) -> DiagnosticBag&;
        template<typename TDiagnosed, typename = std::enable_if_t<std::is_base_of_v<IDiagnosed, TDiagnosed>>>
        auto Add(const TDiagnosed& diagnosed) -> DiagnosticBag&
        {
            return Add(diagnosed.GetDiagnosticBag());
        }
        auto Add(const DiagnosticBag& diagnosticBag) -> DiagnosticBag&;

        auto IsEmpty() const -> bool;
        auto GetDiagnostics() const -> const std::vector<std::shared_ptr<const Diagnostic>>&;
        auto GetSeverity() const -> DiagnosticSeverity;

    private:
        auto AddSeverity(const DiagnosticSeverity& severity) -> void;

        std::vector<std::shared_ptr<const Diagnostic>> m_Diagnostics{};
        DiagnosticSeverity m_Severity = DiagnosticSeverity::Info;
    };
}
