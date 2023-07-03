#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <type_traits>
#include <algorithm>

#include "DiagnosticsBase.hpp"
#include "DiagnosticBag.hpp"
#include "Asserts.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    auto CreateEmptyError() -> std::shared_ptr<const Diagnostic>;

    struct Void
    {
        Void() = default;
        Void(const DiagnosticBag& t_diagnosticBag)
            : DiagnosticBag{ t_diagnosticBag }
        {
        }

        DiagnosticBag DiagnosticBag{};
    };

    template<typename TValue>
    class Expected;

    template<>
    class Expected<void> : public IDiagnosed
    {
    public:
        Expected() = default;
        Expected(
            const Expected& t_other
        ) : m_IsFatal{ t_other.m_IsFatal },
            m_DiagnosticBag{ t_other.m_DiagnosticBag }
        {
        }
        Expected(Expected&& t_other) noexcept
          : m_IsFatal{ t_other.m_IsFatal },
            m_DiagnosticBag{ std::move(t_other.m_DiagnosticBag) }
        {
            t_other.m_IsFatal = false;
        }
        Expected(
            Void&& t_value
        ) : m_DiagnosticBag{ std::move(t_value.DiagnosticBag) }
        {
        }
        Expected(const std::shared_ptr<const Diagnostic>& t_diagnostic)
            : m_IsFatal{ true }
        {
            m_DiagnosticBag.Add(t_diagnostic);
        }
        Expected(const DiagnosticBag& t_bag)
            : m_IsFatal{ true }
        {
            m_DiagnosticBag.Add(t_bag);
        }
        ~Expected() = default;

        auto operator=(const Expected& t_other) -> Expected&
        {
            m_IsFatal = t_other.m_IsFatal;
            m_DiagnosticBag = t_other.m_DiagnosticBag;

            return *this;
        }
        auto operator=(Expected&& t_other) noexcept -> Expected&
        {
            m_IsFatal = t_other.m_IsFatal;
            m_DiagnosticBag = std::move(t_other.m_DiagnosticBag);

            t_other.m_IsFatal = false;

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

    template<typename TValue>
    class Expected : public IDiagnosed
    {
    public:
        Expected()
        {
        }
        Expected(
            const Expected& t_other
        ) : m_OptValue{ t_other.m_OptValue },
            m_DiagnosticBag{ t_other.m_DiagnosticBag }
        {
        }
        Expected(Expected&& t_other) noexcept
          : m_OptValue{ std::move(t_other.m_OptValue) },
            m_DiagnosticBag{ std::move(t_other.m_DiagnosticBag) }
        {
        }
        Expected(
            const TValue& t_value
        ) : m_OptValue{ t_value }
        {
        }
        Expected(
            const TValue& t_value,
            const DiagnosticBag& t_bag
        ) : m_OptValue{ t_value },
            m_DiagnosticBag{ t_bag }
        {
        }
        Expected(TValue&& t_value) noexcept
            : m_OptValue{ std::move(t_value) }
        {
        }
        Expected(
            TValue&& t_value,
            const DiagnosticBag& t_bag
        ) noexcept
          : m_OptValue{ std::move(t_value) },
            m_DiagnosticBag{ t_bag }
        {
        }
        Expected(const std::shared_ptr<const Diagnostic>& t_diagnostic)
        {
            m_DiagnosticBag.Add(t_diagnostic);
        }
        Expected(const DiagnosticBag& t_bag)
        {
            m_DiagnosticBag.Add(t_bag);
        }
        ~Expected() = default;

        auto operator=(const Expected& t_other) -> Expected&
        {
            m_OptValue = t_other.m_OptValue;
            m_DiagnosticBag = t_other.m_DiagnosticBag;

            return *this;
        }
        auto operator=(Expected&& t_other) noexcept -> Expected&
        {
            m_OptValue = std::move(t_other.m_OptValue);
            m_DiagnosticBag = std::move(t_other.m_DiagnosticBag);

            return *this;
        }

        operator bool() const
        {
            return m_OptValue.has_value();
        }

        auto Unwrap() -> TValue&
        {
            return m_OptValue.value();
        }
        auto Unwrap() const -> const TValue&
        {
            return m_OptValue.value();
        }

        auto GetDiagnosticBag() const -> const DiagnosticBag& final
        {
            return m_DiagnosticBag;
        }

        template<typename TValueNew>
        operator Expected<TValueNew>() const
        {
            if (m_DiagnosticBag.IsEmpty())
            {
                return Expected<TValueNew>(m_OptValue.value());
            }
            else
            {
                return Expected<TValueNew>(m_DiagnosticBag);
            }
        }

    private:
        std::optional<TValue> m_OptValue{};
        DiagnosticBag m_DiagnosticBag{};
    };

#define TIn typename std::decay_t<decltype(*TIt{})>
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap())>

    template<typename TIt, typename F>
    auto TransformExpected(
        TIt t_begin,
        TIt t_end,
        F&& t_func
    ) -> Expected<std::vector<TOut>>
    {
        std::vector<TOut> vec{};
        vec.reserve(std::distance(t_begin, t_end));

        ACE_TRY_ASSERT(std::find_if_not(t_begin, t_end,
        [&](const TIn& t_element)
        {
            auto expOut = t_func(t_element);
            if (!expOut)
            {
                return false;
            }

            if constexpr (std::is_move_constructible_v<TOut>)
            {
                vec.push_back(std::move(expOut.Unwrap()));
            }
            else
            {
                vec.push_back(expOut.Unwrap());
            }

            return true;
        }) == t_end);

        return vec;
    }

#undef TIn
#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedVector(
        const std::vector<TIn>& t_inVec,
        F&& t_func
    ) -> std::enable_if_t<!std::is_same_v<TOut, void>, Expected<std::vector<TOut>>>
    {
        std::vector<TOut> outVec{};
        outVec.reserve(t_inVec.size());

        ACE_TRY_ASSERT(std::find_if_not(begin(t_inVec), end(t_inVec),
        [&](const TIn& t_element)
        {
            auto expOut = t_func(t_element);
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

        }) == end(t_inVec));

        return outVec;
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedVector(
        const std::vector<TIn>& t_inVec,
        F&& t_func
    ) -> std::enable_if_t<std::is_same_v<TOut, void>, Expected<void>>
    {
        ACE_TRY_ASSERT(std::find_if_not(begin(t_inVec), end(t_inVec),
        [&](const TIn& t_element)
        {
            return t_func(t_element);
        }) == end(t_inVec));

        return Void{};
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedOptional(
        const std::optional<TIn>& t_optIn,
        F&& t_func
    ) -> Expected<std::optional<TOut>>
    {
        if (!t_optIn.has_value())
        {
            return std::optional<TOut>{};
        }

        ACE_TRY(out, t_func(t_optIn.value()));
        return std::optional{ out };
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap().Value)>

    template<typename TIn, typename F>
    auto TransformExpectedMaybeChangedVector(
        const std::vector<TIn>& t_inVec,
        F&& t_func
    ) -> Expected<MaybeChanged<std::vector<TOut>>>
    {
        bool isChanged = false;
        std::vector<TOut> outVec{};
        outVec.reserve(t_inVec.size());

        ACE_TRY_ASSERT(std::find_if_not(begin(t_inVec), end(t_inVec),
        [&](const TIn& t_element)
        {
            auto expMchElement = t_func(t_element);
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

        }) == end(t_inVec));

        if (!isChanged)
        {
            return CreateUnchanged(t_inVec);
        }

        return CreateChanged(outVec);
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap().Value)>

    template<typename TIn, typename F>
    auto TransformExpectedMaybeChangedOptional(
        const std::optional<TIn>& t_optIn,
        F&& t_func
    ) -> Expected<MaybeChanged<std::optional<TOut>>>
    {
        bool isChanged = false;

        if (!t_optIn.has_value())
        {
            return CreateUnchanged(t_optIn);
        }

        ACE_TRY(mchOut, t_func(t_optIn.value()));

        if (!mchOut.IsChanged)
        {
            return CreateUnchanged(t_optIn);
        }

        return CreateChanged(std::optional{ mchOut.Value });
    }

#undef TOut
}
