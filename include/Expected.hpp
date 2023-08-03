#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <type_traits>
#include <algorithm>

#include "DiagnosticBase.hpp"
#include "DiagnosticBag.hpp"
#include "Assert.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    struct EmptyError
    {
    };

    auto CreateEmptyError() -> std::shared_ptr<const DiagnosticGroup>;

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
    class [[nodiscard]] Expected<void> : public IDiagnosed
    {
    public:
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
            const EmptyError& emptyError
        ) : m_IsFatal{ true }
        {
            m_Diagnostics.Add(CreateEmptyError());
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

        auto GetDiagnosticBag() const -> const DiagnosticBag&
        {
            return m_Diagnostics;
        }

    private:
        bool m_IsFatal{};
        DiagnosticBag m_Diagnostics{};
    };

    template<typename T>
    class [[nodiscard]] Expected : public IDiagnosed
    {
    public:
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
        Expected(const EmptyError& error)
        {
            m_Diagnostics.Add(CreateEmptyError());
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
        auto UnwrapOr(T value) const -> T
        {
            if (m_OptValue.has_value())
            {
                return m_OptValue.value();
            }
            else
            {
                return value;
            }
        }

        auto GetDiagnosticBag() const -> const DiagnosticBag& final
        {
            return m_Diagnostics;
        }

        template<typename TNew>
        operator Expected<TNew>() const
        {
            if (m_Diagnostics.IsEmpty())
            {
                return Expected<TNew>(m_OptValue.value());
            }
            else
            {
                return Expected<TNew>(m_Diagnostics);
            }
        }

    private:
        std::optional<T> m_OptValue{};
        DiagnosticBag m_Diagnostics{};
    };

#define TIn  typename std::decay_t<decltype(*TBegin{})>
#define TOut typename std::decay_t<decltype(func(TIn{}).Unwrap())>

    template<typename TBegin, typename TEnd, typename F>
    auto TransformExpected(
        const TBegin begin,
        const TEnd end,
        F&& func
    ) -> std::enable_if_t<!std::is_same_v<TOut, void>, Expected<std::vector<TOut>>>
    {
        DiagnosticBag diagnostics{};

        std::vector<TOut> outVec{};
        const auto unexpectedIt = std::find_if_not(begin, end,
        [&](const TIn& element)
        {
            auto expOut = func(element);
            diagnostics.Add(expOut);
            if (!expOut)
            {
                return false;
            }

            if constexpr (std::is_move_constructible_v<TOut>)
            {
                outVec.push_back(std::move(expOut.Unwrap()));
            }
            else
            {
                outVec.push_back(expOut.Unwrap());
            }

            return true;
        });

        if (unexpectedIt != end)
        {
            return diagnostics;
        }

        return { outVec, diagnostics };
    }

#undef TOut
#define TIn  typename std::decay_t<decltype(*TBegin{})>
#define TOut typename std::decay_t<decltype(func(*TIn{}).Unwrap())>

    template<typename TBegin, typename TEnd, typename F>
    auto TransformExpected(
        const TBegin begin,
        const TEnd end,
        F&& func
    ) -> std::enable_if_t<std::is_same_v<TOut, void>, Expected<void>>
    {
        DiagnosticBag diagnostics{};

        const auto unexpectedIt = std::find_if_not(begin, end,
        [&](const TIn& element)
        {
            const auto expOut = func(element);
            diagnostics.Add(expOut);
            if (!expOut)
            {
                return false;
            }

            return true;
        });

        if (unexpectedIt != end)
        {
            return diagnostics;
        }

        return Void{ diagnostics };
    }

#undef TIn
#undef TOut
#define TOut typename std::decay_t<decltype(func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedVector(
        const std::vector<TIn>& inVec,
        F&& func
    ) -> std::enable_if_t<!std::is_same_v<TOut, void>, Expected<std::vector<TOut>>>
    {
        DiagnosticBag diagnostics{};

        std::vector<TOut> outVec{};
        outVec.reserve(inVec.size());

        const auto unexpectedIt = std::find_if_not(
            begin(inVec),
            end  (inVec),
            [&](const TIn& element)
            {
                auto expOut = func(element);
                diagnostics.Add(expOut);
                if (!expOut)
                {
                    return false;
                }

                if constexpr (std::is_move_constructible_v<TOut>)
                {
                    outVec.push_back(std::move(expOut.Unwrap()));
                }
                else
                {
                    outVec.push_back(expOut.Unwrap());
                }

                return true;
            }
        );

        if (unexpectedIt != end(inVec))
        {
            return diagnostics;
        }

        return { outVec, diagnostics };
    }

#undef TOut
#define TOut typename std::decay_t<decltype(func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedVector(
        const std::vector<TIn>& inVec,
        F&& func
    ) -> std::enable_if_t<std::is_same_v<TOut, void>, Expected<void>>
    {
        DiagnosticBag diagnostics{};

        const auto unexpectedIt = std::find_if_not(
            begin(inVec),
            end  (inVec),
            [&](const TIn& element)
            {
                const auto expOut = func(element);
                diagnostics.Add(expOut);
                if (!expOut)
                {
                    return false;
                }

                return true;
            }
        );

        if (unexpectedIt != end(inVec))
        {
            return diagnostics;
        }

        return Void{ diagnostics };
    }

#undef TOut
#define TOut typename std::decay_t<decltype(func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedOptional(
        const std::optional<TIn>& optIn,
        F&& func
    ) -> Expected<std::optional<TOut>>
    {
        if (!optIn.has_value())
        {
            return std::optional<TOut>{};
        }

        ACE_TRY(out, func(optIn.value()));
        return std::optional{ out };
    }

#undef TOut
#define TOut typename std::decay_t<decltype(func(TIn{}).Unwrap().Value)>

    template<typename TIn, typename F>
    auto TransformExpectedCacheableVector(
        const std::vector<TIn>& inVec,
        F&& func
    ) -> Expected<Cacheable<std::vector<TOut>>>
    {
        DiagnosticBag diagnostics{};

        bool isChanged = false;
        std::vector<TOut> outVec{};
        outVec.reserve(inVec.size());

        const auto unexpectedIt = std::find_if_not(
            begin(inVec),
            end  (inVec),
            [&](const TIn& element)
            {
                auto expCchElement = func(element);
                diagnostics.Add(expCchElement);
                if (!expCchElement)
                {
                    return false;
                }

                if (expCchElement.Unwrap().IsChanged)
                {
                    isChanged = true;
                }

                if constexpr (std::is_move_constructible_v<TOut>)
                {
                    outVec.push_back(std::move(expCchElement.Unwrap().Value));
                }
                else
                {
                    outVec.push_back(expCchElement.Unwrap().Value);
                }

                return true;
            }
        );

        if (unexpectedIt != end(inVec))
        {
            return diagnostics;
        }

        if (!isChanged)
        {
            return CreateUnchanged(inVec);
        }

        return CreateChanged(outVec);
    }

#undef TOut
#define TOut typename std::decay_t<decltype(func(TIn{}).Unwrap().Value)>

    template<typename TIn, typename F>
    auto TransformExpectedCacheableOptional(
        const std::optional<TIn>& optIn,
        F&& func
    ) -> Expected<Cacheable<std::optional<TOut>>>
    {
        bool isChanged = false;

        if (!optIn.has_value())
        {
            return CreateUnchanged(optIn);
        }

        ACE_TRY(cchOut, func(optIn.value()));

        if (!cchOut.IsChanged)
        {
            return CreateUnchanged(optIn);
        }

        return CreateChanged(std::optional{ cchOut.Value });
    }

#undef TOut
}
