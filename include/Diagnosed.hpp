#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "DiagnosticsBase.hpp"
#include "DiagnosticBag.hpp"

namespace Ace
{
    template<typename TValue, typename TDiagnostic>
    class Diagnosed;

    template<typename TDiagnostic>
    class Diagnosed<void, TDiagnostic>
    {
        static_assert(
            std::is_base_of_v<IDiagnostic, TDiagnostic>,
            "TDiagnostic must be derived from IDiagnostic"
        );

    public:
        Diagnosed() = default;
        Diagnosed(
            const DiagnosticBag<TDiagnostic>& t_diagnosticBag
        ) : m_DiagnosticBag{ t_diagnosticBag }
        {
        }

        auto GetDiagnosticBag() const -> const DiagnosticBag<TDiagnostic>&
        {
            return m_DiagnosticBag;
        }

    private:
        DiagnosticBag<TDiagnostic> m_DiagnosticBag{};
    };

    template<typename TValue, typename TDiagnostic>
    class Diagnosed
    {
        static_assert(
            std::is_base_of_v<IDiagnostic, TDiagnostic>,
            "TDiagnostic must be derived from IDiagnostic"
        );

    public:
        Diagnosed(
            const TValue& t_value
        ) : m_Value{ t_value }
        {
        }
        Diagnosed(
            const TValue& t_value,
            const DiagnosticBag<TDiagnostic>& t_diagnosticBag
        ) : m_Value{ t_value },
            m_DiagnosticBag{ t_diagnosticBag }
        {
        }
        Diagnosed(
            TValue&& t_value,
            const DiagnosticBag<TDiagnostic>& t_diagnosticBag
        ) : m_Value{ std::move(t_value) },
            m_DiagnosticBag{ t_diagnosticBag }
        {
        }

        auto Unwrap() -> TValue&
        {
            return m_Value;
        }
        auto Unwrap() const -> const TValue&
        {
            return m_Value;
        }

        auto GetDiagnosticBag() const -> const DiagnosticBag<TDiagnostic>&
        {
            return m_DiagnosticBag;
        }

    private:
        TValue m_Value{};
        DiagnosticBag<TDiagnostic> m_DiagnosticBag{};
    };
}
