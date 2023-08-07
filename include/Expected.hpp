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
        Void() = default;
        Void(
            const DiagnosticBag& diagnostics
        ) : DiagnosticBag{ diagnostics }
        {
        }

        DiagnosticBag DiagnosticBag{};
    };

    template<typename T>
    class [[nodiscard]] Expected;

    template<>
    class [[nodiscard]] Expected<void>
    {
    public:
        [[deprecated]]
        auto _ExpectedVoid() -> void {  }

        Expected() = default;
        Expected(
            const Expected& other
        ) : m_IsFatal{ other.m_IsFatal },
            m_Diagnostics{ other.m_Diagnostics }
        {
        }
        Expected(Expected&& other) noexcept
          : m_IsFatal{ other.m_IsFatal },
            m_Diagnostics{ std::move(other.m_Diagnostics) }
        {
            other.m_IsFatal = false;
        }
        Expected(
            Void&& value
        ) : m_Diagnostics{ std::move(value.DiagnosticBag) }
        {
        }
        Expected(
            const DiagnosticBag& diagnostics
        ) : m_IsFatal{ true }
        {
            m_Diagnostics.Add(diagnostics);
        }
        ~Expected() = default;

        auto operator=(const Expected& other) -> Expected&
        {
            m_IsFatal = other.m_IsFatal;
            m_Diagnostics = other.m_Diagnostics;

            return *this;
        }
        auto operator=(Expected&& other) noexcept -> Expected&
        {
            m_IsFatal = other.m_IsFatal;
            m_Diagnostics = std::move(other.m_Diagnostics);

            other.m_IsFatal = false;

            return *this;
        }

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

    private:
        bool m_IsFatal{};
        DiagnosticBag m_Diagnostics{};
    };

    template<typename T>
    class [[nodiscard]] Expected
    {
    public:
        [[deprecated]]
        auto _ExpectedNotVoid() -> void {  }

        Expected()
        {
        }
        Expected(
            const Expected& other
        ) : m_OptValue{ other.m_OptValue },
            m_Diagnostics{ other.m_Diagnostics }
        {
        }
        Expected(Expected&& other) noexcept
          : m_OptValue{ std::move(other.m_OptValue) },
            m_Diagnostics{ std::move(other.m_Diagnostics) }
        {
        }
        Expected(
            const T& value
        ) : m_OptValue{ value }
        {
        }
        Expected(
            const T& value,
            const DiagnosticBag& diagnostics
        ) : m_OptValue{ value },
            m_Diagnostics{ diagnostics }
        {
        }
        Expected(T&& value) noexcept
            : m_OptValue{ std::move(value) }
        {
        }
        Expected(
            T&& value,
            const DiagnosticBag& diagnostics
        ) noexcept
          : m_OptValue{ std::move(value) },
            m_Diagnostics{ diagnostics }
        {
        }
        Expected(const DiagnosticBag& diagnostics)
        {
            m_Diagnostics.Add(diagnostics);
        }
        ~Expected() = default;

        auto operator=(const Expected& other) -> Expected&
        {
            m_OptValue = other.m_OptValue;
            m_Diagnostics = other.m_Diagnostics;

            return *this;
        }
        auto operator=(Expected&& other) noexcept -> Expected&
        {
            m_OptValue = std::move(other.m_OptValue);
            m_Diagnostics = std::move(other.m_Diagnostics);

            return *this;
        }

        template<typename TNew>
        operator Expected<TNew>() const
        {
            if (m_OptValue.has_value())
            {
                return Expected<TNew>{ m_OptValue.value(), m_Diagnostics };
            }
            else
            {
                return Expected<TNew>{ m_Diagnostics };
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

    private:
        std::optional<T> m_OptValue{};
        DiagnosticBag m_Diagnostics{};
    };
}
