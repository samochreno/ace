#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "Assert.hpp"
#include "DiagnosticBase.hpp"

namespace Ace
{
    template<typename T> inline constexpr bool IsDiagnosedVoid = 
        requires(T t) { t._DiagnosedVoid(); };

    template<typename T> inline constexpr bool IsDiagnosedNotVoid = 
        requires(T t) { t._DiagnosedNotVoid(); };

    template<typename T> inline constexpr bool IsExpectedVoid = 
        requires(T t) { t._ExpectedVoid(); };

    template<typename T> inline constexpr bool IsExpectedNotVoid = 
        requires(T t) { t._ExpectedNotVoid(); };

    class DiagnosticBag
    {
    public:
        DiagnosticBag() = default;
        ~DiagnosticBag() = default;

        auto Add(
            const std::shared_ptr<const DiagnosticGroup>& diagnosticGroup
        ) -> DiagnosticBag&;
        auto Add(const DiagnosticBag& diagnostics) -> DiagnosticBag&;

        template<typename TDiagnosed>
        auto Collect(
            TDiagnosed diagnosed
        ) -> void requires IsDiagnosedVoid<TDiagnosed>
        {
            Add(diagnosed.GetDiagnostics());
        }

#define TValue typename std::decay_t<decltype(diagnosed.Unwrap())>

        template<typename TDiagnosed>
        [[nodiscard]]
        auto Collect(
            TDiagnosed diagnosed
        ) -> TValue requires IsDiagnosedNotVoid<TDiagnosed>
        {
            Add(diagnosed.GetDiagnostics());
            return std::move(diagnosed.Unwrap());
        }

#undef TValue

        template<typename TExpected>
        auto Collect(
            TExpected expected
        ) -> bool requires IsExpectedVoid<TExpected>
        {
            Add(expected.GetDiagnostics());
            return expected;
        }

#define TValue typename std::decay_t<decltype(expected.Unwrap())>

        template<typename TExpected>
        [[nodiscard]]
        auto Collect(
            TExpected expected
        ) -> std::optional<TValue> requires IsExpectedNotVoid<TExpected>
        {
            Add(expected.GetDiagnostics());
            if (!expected)
            {
                return std::nullopt;
            }

            return std::optional{ std::move(expected.Unwrap()) };
        }

#undef TValue

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
