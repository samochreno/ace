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
    class [[nodiscard]] Diagnosed<void>
    {
    public:
        [[deprecated]]
        auto _DiagnosedVoid() -> void {}

        Diagnosed([[deprecated]] const Diagnosed&) = default;
        Diagnosed(
            DiagnosticBag diagnostics
        ) : m_Diagnostics{ std::move(diagnostics) }
        {
        }
        ~Diagnosed() = default;

        auto GetDiagnostics() const -> const DiagnosticBag&
        {
            return m_Diagnostics;
        }
        auto GetDiagnostics() -> DiagnosticBag&
        {
            return m_Diagnostics;
        }

    private:
        DiagnosticBag m_Diagnostics = DiagnosticBag::Create();
    };

    template<typename TValue>
    class [[nodiscard]] Diagnosed
    {
    public:
        [[deprecated]]
        auto _DiagnosedNotVoid() -> void {}

        Diagnosed([[deprecated]] const Diagnosed&) = default;
        Diagnosed(
            const TValue& value,
            DiagnosticBag diagnostics
        ) : m_Value{ value },
            m_Diagnostics{ std::move(diagnostics) }
        {
        }
        Diagnosed(
            TValue&& value,
            DiagnosticBag diagnostics
        ) : m_Value{ std::move(value) },
            m_Diagnostics{ std::move(diagnostics) }
        {
        }
        ~Diagnosed() = default;

        template<typename TNew>
        operator Diagnosed<TNew>() &&
        {
            return Diagnosed<TNew>
            {
                std::move(m_Value),
                std::move(m_Diagnostics),
            };
        }

        auto Unwrap() -> TValue&
        {
            return m_Value;
        }
        auto Unwrap() const -> const TValue&
        {
            return m_Value;
        }

        auto GetDiagnostics() const -> const DiagnosticBag&
        {
            return m_Diagnostics;
        }
        auto GetDiagnostics() -> DiagnosticBag&
        {
            return m_Diagnostics;
        }

    private:
        TValue m_Value{};
        DiagnosticBag m_Diagnostics = DiagnosticBag::Create();
    };
}
