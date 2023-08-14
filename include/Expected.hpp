#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <type_traits>
#include <algorithm>

#include "DiagnosticBase.hpp"
#include "DiagnosticBag.hpp"
#include "Assert.hpp"

namespace Ace
{
    struct Void
    {
        Void(
            DiagnosticBag diagnostics
        ) : Diagnostics{ std::move(diagnostics) }
        {
        }

        DiagnosticBag Diagnostics = DiagnosticBag::Create();
    };

    template<typename T>
    class [[nodiscard]] Expected;

    template<>
    class [[nodiscard]] Expected<void>
    {
    public:
        [[deprecated]]
        auto _ExpectedVoid() -> void {}

        Expected() = default;
        Expected(const Expected&) = delete;
        Expected(
            Void value
        ) : m_Diagnostics{ std::move(value.Diagnostics) }
        {
        }
        Expected(
            DiagnosticBag diagnostics
        ) : m_IsFatal{ true }
        {
            m_Diagnostics.Add(std::move(diagnostics));
        }
        ~Expected() = default;

        operator bool() const
        {
            return !m_IsFatal;
        }

        auto Unwrap() -> void
        {
            if (!m_IsFatal)
            {
                return;
            }

            ACE_UNREACHABLE();
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
        bool m_IsFatal{};
        DiagnosticBag m_Diagnostics = DiagnosticBag::Create();
    };

    template<typename T>
    class [[nodiscard]] Expected
    {
    public:
        [[deprecated]]
        auto _ExpectedNotVoid() -> void {  }

        Expected() = default;
        Expected(const Expected&) = delete;
        Expected(Expected&&) = default;
        Expected(
            const T& value,
            DiagnosticBag diagnostics
        ) : m_OptValue{ value },
            m_Diagnostics{ std::move(diagnostics) }
        {
        }
        Expected(
            T&& value,
            DiagnosticBag diagnostics
        ) noexcept
          : m_OptValue{ std::move(value) },
            m_Diagnostics{ std::move(diagnostics) }
        {
        }
        Expected(DiagnosticBag diagnostics)
        {
            m_Diagnostics.Add(std::move(diagnostics));
        }
        ~Expected() = default;

        template<typename TNew>
        operator Expected<TNew>() &&
        {
            if (m_OptValue.has_value())
            {
                return Expected<TNew>
                {
                    std::move(m_OptValue.value()),
                    std::move(m_Diagnostics),
                };
            }
            else
            {
                return Expected<TNew>{ std::move(m_Diagnostics) };
            }
        }

        operator bool() const
        {
            return m_OptValue.has_value();
        }

        auto Unwrap() -> T&
        {
            return m_OptValue.value();
        }
        auto Unwrap() const -> const T&
        {
            return m_OptValue.value();
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
        std::optional<T> m_OptValue{};
        DiagnosticBag m_Diagnostics = DiagnosticBag::Create();
    };
}
