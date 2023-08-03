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
            const std::shared_ptr<const DiagnosticGroup>& diagnosticGroup
        ) -> DiagnosticBag&;
        template<typename TDiagnosed, typename = std::enable_if_t<std::is_base_of_v<IDiagnosed, TDiagnosed>>>
        auto Add(const TDiagnosed& diagnosed) -> DiagnosticBag&
        {
            return Add(diagnosed.GetDiagnosticBag());
        }
        auto Add(const DiagnosticBag& diagnostics) -> DiagnosticBag&;

        auto IsEmpty() const -> bool;
        auto GetDiagnosticGroups() const -> const std::vector<std::shared_ptr<const DiagnosticGroup>>&;
        auto GetSeverity() const -> DiagnosticSeverity;
        auto HasErrors() const -> bool;

    private:
        auto AddSeverity(const DiagnosticSeverity& severity) -> void;

        std::vector<std::shared_ptr<const DiagnosticGroup>> m_DiagnosticGroups{};
        DiagnosticSeverity m_Severity = DiagnosticSeverity::Info;
    };
}
