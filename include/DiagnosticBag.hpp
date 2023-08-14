#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>
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
        ~DiagnosticBag() = default;
        
        static auto Create()        -> DiagnosticBag;
        static auto CreateNoError() -> DiagnosticBag;
        static auto CreateGlobal()  -> DiagnosticBag;

        auto Add(DiagnosticBag diagnosticBag) -> DiagnosticBag&;
        auto Add(DiagnosticGroup diagnosticGroup) -> DiagnosticBag&;

        template<typename TDiagnosed>
        auto Collect(TDiagnosed value) -> void requires IsDiagnosedVoid<TDiagnosed>
        {
            Add(std::move(value.GetDiagnostics()));
        }
        template<typename TDiagnosed>
        [[nodiscard]]
        auto Collect(TDiagnosed value) requires IsDiagnosedNotVoid<TDiagnosed>
        {
            Add(std::move(value.GetDiagnostics()));
            return std::move(value.Unwrap());
        }
        template<typename TExpected>
        auto Collect(TExpected value) -> bool requires IsExpectedVoid<TExpected>
        {
            Add(std::move(value.GetDiagnostics()));
            return value;
        }
        template<typename TExpected>
        [[nodiscard]]
        auto Collect(TExpected value) requires IsExpectedNotVoid<TExpected>
        {
            Add(std::move(value.GetDiagnostics()));
            if (!value)
            {
                return std::optional<std::decay_t<decltype(value.Unwrap())>>{};
            }

            return std::optional{ std::move(value.Unwrap()) };
        }

        auto IsEmpty() const -> bool;
        auto GetSeverity() const -> DiagnosticSeverity;
        auto HasErrors() const -> bool;

    private:
        DiagnosticBag() = default;

        auto AddSeverity(const DiagnosticSeverity severity) -> void;

        std::optional<std::function<void(const DiagnosticGroup&)>> m_OptHandler{};
        std::vector<DiagnosticGroup> m_DiagnosticGroups{};
        DiagnosticSeverity m_Severity = DiagnosticSeverity::Info;
    };
}
