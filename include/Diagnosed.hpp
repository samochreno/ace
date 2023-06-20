#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "DiagnosticsBase.hpp"
#include "DiagnosticBag.hpp"

namespace Ace
{
    template<typename TValue>
    class Diagnosed;

    template<>
    class Diagnosed<void>
    {
    public:
        Diagnosed() = default;
        Diagnosed(
            const DiagnosticBag& t_diagnosticBag
        ) : m_DiagnosticBag{ t_diagnosticBag }
        {
        }

        auto GetDiagnosticBag() const -> const DiagnosticBag&
        {
            return m_DiagnosticBag;
        }

    private:
        DiagnosticBag m_DiagnosticBag{};
    };

    template<typename TValue>
    class Diagnosed
    {
    public:
        Diagnosed(
            const TValue& t_value
        ) : m_Value{ t_value }
        {
        }
        Diagnosed(
            const TValue& t_value,
            const DiagnosticBag& t_diagnosticBag
        ) : m_Value{ t_value },
            m_DiagnosticBag{ t_diagnosticBag }
        {
        }
        Diagnosed(
            TValue&& t_value,
            const DiagnosticBag& t_diagnosticBag
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

        auto GetDiagnosticBag() const -> const DiagnosticBag&
        {
            return m_DiagnosticBag;
        }

    private:
        TValue m_Value{};
        DiagnosticBag m_DiagnosticBag{};
    };
}
