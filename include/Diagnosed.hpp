#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "DiagnosticBase.hpp"
#include "DiagnosticBag.hpp"

namespace Ace
{
    template<typename TValue>
    class Diagnosed;

    template<>
    class Diagnosed<void> : public IDiagnosed
    {
    public:
        Diagnosed() = default;
        Diagnosed(
            const DiagnosticBag& diagnosticBag
        ) : m_DiagnosticBag{ diagnosticBag }
        {
        }

        auto GetDiagnosticBag() const -> const DiagnosticBag& final
        {
            return m_DiagnosticBag;
        }

    private:
        DiagnosticBag m_DiagnosticBag{};
    };

    template<typename TValue>
    class Diagnosed : public IDiagnosed
    {
    public:
        Diagnosed(
            const TValue& value,
            const DiagnosticBag& diagnosticBag
        ) : m_Value{ value },
            m_DiagnosticBag{ diagnosticBag }
        {
        }
        Diagnosed(
            TValue&& value,
            const DiagnosticBag& diagnosticBag
        ) : m_Value{ std::move(value) },
            m_DiagnosticBag{ diagnosticBag }
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

        auto GetDiagnosticBag() const -> const DiagnosticBag& final
        {
            return m_DiagnosticBag;
        }

    private:
        TValue m_Value{};
        DiagnosticBag m_DiagnosticBag{};
    };
}
