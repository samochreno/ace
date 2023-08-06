#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "DiagnosticBase.hpp"
#include "DiagnosticBag.hpp"

namespace Ace
{
    template<typename TValue>
    class [[nodiscard]] Diagnosed;

    template<>
    class [[nodiscard]] Diagnosed<void> : public IDiagnosed

    {
    public:
        Diagnosed() = default;
        Diagnosed(
            const DiagnosticBag& diagnostics
        ) : m_Diagnostics{ diagnostics }
        {
        }

        auto GetDiagnostics() const -> const DiagnosticBag& final
        {
            return m_Diagnostics;
        }

    private:
        DiagnosticBag m_Diagnostics{};
    };

    template<typename TValue>
    class [[nodiscard]] Diagnosed : public IDiagnosed
    {
    public:
        Diagnosed(
            const TValue& value,
            const DiagnosticBag& diagnostics
        ) : m_Value{ value },
            m_Diagnostics{ diagnostics }
        {
        }
        Diagnosed(
            TValue&& value,
            const DiagnosticBag& diagnostics
        ) : m_Value{ std::move(value) },
            m_Diagnostics{ diagnostics }
        {
        }

        template<typename TNew>
        operator Diagnosed<TNew>() const
        {
            return Diagnosed<TNew>{ m_Value, m_Diagnostics };
        }

        auto Unwrap() -> TValue&
        {
            return m_Value;
        }
        auto Unwrap() const -> const TValue&
        {
            return m_Value;
        }

        auto GetDiagnostics() const -> const DiagnosticBag& final
        {
            return m_Diagnostics;
        }

    private:
        TValue m_Value{};
        DiagnosticBag m_Diagnostics{};
    };
}
