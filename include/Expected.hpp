#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <type_traits>
#include <algorithm>

#include "DiagnosticBase.hpp"
#include "DiagnosticBag.hpp"
#include "Assert.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    auto CreateEmptyError() -> std::shared_ptr<const Diagnostic>;

    struct Void
    {
        Void() = default;
        Void(
            const DiagnosticBag& diagnosticBag
        ) : DiagnosticBag{ diagnosticBag }
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
            m_DiagnosticBag{ other.m_DiagnosticBag }
        {
        }
        Expected(Expected&& other) noexcept
          : m_IsFatal{ other.m_IsFatal },
            m_DiagnosticBag{ std::move(other.m_DiagnosticBag) }
        {
            other.m_IsFatal = false;
        }
        Expected(
            Void&& value
        ) : m_DiagnosticBag{ std::move(value.DiagnosticBag) }
        {
        }
        Expected(
            const std::shared_ptr<const Diagnostic>& diagnostic
        ) : m_IsFatal{ true }
        {
            m_DiagnosticBag.Add(diagnostic);
        }
        Expected(
            const DiagnosticBag& diagnosticBag
        ) : m_IsFatal{ true }
        {
            m_DiagnosticBag.Add(diagnosticBag);
        }
        ~Expected() = default;

        auto operator=(const Expected& other) -> Expected&
        {
            m_IsFatal = other.m_IsFatal;
            m_DiagnosticBag = other.m_DiagnosticBag;

            return *this;
        }
        auto operator=(Expected&& other) noexcept -> Expected&
        {
            m_IsFatal = other.m_IsFatal;
            m_DiagnosticBag = std::move(other.m_DiagnosticBag);

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
            return m_DiagnosticBag;
        }

    private:
        bool m_IsFatal{};
        DiagnosticBag m_DiagnosticBag{};
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
            m_DiagnosticBag{ other.m_DiagnosticBag }
        {
        }
        Expected(Expected&& other) noexcept
          : m_OptValue{ std::move(other.m_OptValue) },
            m_DiagnosticBag{ std::move(other.m_DiagnosticBag) }
        {
        }
        Expected(
            const T& value
        ) : m_OptValue{ value }
        {
        }
        Expected(
            const T& value,
            const DiagnosticBag& diagnosticBag
        ) : m_OptValue{ value },
            m_DiagnosticBag{ diagnosticBag }
        {
        }
        Expected(T&& value) noexcept
            : m_OptValue{ std::move(value) }
        {
        }
        Expected(
            T&& value,
            const DiagnosticBag& diagnosticBag
        ) noexcept
          : m_OptValue{ std::move(value) },
            m_DiagnosticBag{ diagnosticBag }
        {
        }
        Expected(const std::shared_ptr<const Diagnostic>& diagnostic)
        {
            m_DiagnosticBag.Add(diagnostic);
        }
        Expected(const DiagnosticBag& diagnosticBag)
        {
            m_DiagnosticBag.Add(diagnosticBag);
        }
        ~Expected() = default;

        auto operator=(const Expected& other) -> Expected&
        {
            m_OptValue = other.m_OptValue;
            m_DiagnosticBag = other.m_DiagnosticBag;

            return *this;
        }
        auto operator=(Expected&& other) noexcept -> Expected&
        {
            m_OptValue = std::move(other.m_OptValue);
            m_DiagnosticBag = std::move(other.m_DiagnosticBag);

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
            return m_DiagnosticBag;
        }

        template<typename TNew>
        operator Expected<TNew>() const
        {
            if (m_DiagnosticBag.IsEmpty())
            {
                return Expected<TNew>(m_OptValue.value());
            }
            else
            {
                return Expected<TNew>(m_DiagnosticBag);
            }
        }

    private:
        std::optional<T> m_OptValue{};
        DiagnosticBag m_DiagnosticBag{};
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
        DiagnosticBag diagnosticBag{};

        std::vector<TOut> outVec{};
        const auto unexpectedIt = std::find_if_not(begin, end,
        [&](const TIn& element)
        {
            auto expOut = func(element);
            diagnosticBag.Add(expOut);
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
            return diagnosticBag;
        }

        return { outVec, diagnosticBag };
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
        DiagnosticBag diagnosticBag{};

        const auto unexpectedIt = std::find_if_not(begin, end,
        [&](const TIn& element)
        {
            const auto expOut = func(element);
            diagnosticBag.Add(expOut);
            if (!expOut)
            {
                return false;
            }

            return true;
        });

        if (unexpectedIt != end)
        {
            return diagnosticBag;
        }

        return Void{ diagnosticBag };
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
        DiagnosticBag diagnosticBag{};

        std::vector<TOut> outVec{};
        outVec.reserve(inVec.size());

        const auto unexpectedIt = std::find_if_not(
            begin(inVec),
            end  (inVec),
            [&](const TIn& element)
            {
                auto expOut = func(element);
                diagnosticBag.Add(expOut);
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
            return diagnosticBag;
        }

        return { outVec, diagnosticBag };
    }

#undef TOut
#define TOut typename std::decay_t<decltype(func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedVector(
        const std::vector<TIn>& inVec,
        F&& func
    ) -> std::enable_if_t<std::is_same_v<TOut, void>, Expected<void>>
    {
        DiagnosticBag diagnosticBag{};

        const auto unexpectedIt = std::find_if_not(
            begin(inVec),
            end  (inVec),
            [&](const TIn& element)
            {
                const auto expOut = func(element);
                diagnosticBag.Add(expOut);
                if (!expOut)
                {
                    return false;
                }

                return true;
            }
        );

        if (unexpectedIt != end(inVec))
        {
            return diagnosticBag;
        }

        return Void{ diagnosticBag };
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
    auto TransformExpectedMaybeChangedVector(
        const std::vector<TIn>& inVec,
        F&& func
    ) -> Expected<MaybeChanged<std::vector<TOut>>>
    {
        DiagnosticBag diagnosticBag{};

        bool isChanged = false;
        std::vector<TOut> outVec{};
        outVec.reserve(inVec.size());

        const auto unexpectedIt = std::find_if_not(
            begin(inVec),
            end  (inVec),
            [&](const TIn& element)
            {
                auto expMchElement = func(element);
                diagnosticBag.Add(expMchElement);
                if (!expMchElement)
                {
                    return false;
                }

                if (expMchElement.Unwrap().IsChanged)
                {
                    isChanged = true;
                }

                if constexpr (std::is_move_constructible_v<TOut>)
                {
                    outVec.push_back(std::move(expMchElement.Unwrap().Value));
                }
                else
                {
                    outVec.push_back(expMchElement.Unwrap().Value);
                }

                return true;
            }
        );

        if (unexpectedIt != end(inVec))
        {
            return diagnosticBag;
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
    auto TransformExpectedMaybeChangedOptional(
        const std::optional<TIn>& optIn,
        F&& func
    ) -> Expected<MaybeChanged<std::optional<TOut>>>
    {
        bool isChanged = false;

        if (!optIn.has_value())
        {
            return CreateUnchanged(optIn);
        }

        ACE_TRY(mchOut, func(optIn.value()));

        if (!mchOut.IsChanged)
        {
            return CreateUnchanged(optIn);
        }

        return CreateChanged(std::optional{ mchOut.Value });
    }

#undef TOut
}
