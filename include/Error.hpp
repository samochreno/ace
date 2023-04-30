#pragma once

#include <vector>
#include <optional>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <functional>

#include "Asserts.hpp"
#include "MaybeChanged.hpp"

#pragma warning(push)
#pragma warning(disable: 26495) // Always initialize a member variable.

#define ACE_MACRO_CONCAT_IMPL(t_x, t_y) t_x##t_y
#define ACE_MACRO_CONCAT(t_x, t_y) ACE_MACRO_CONCAT_IMPL(t_x, t_y)

#define ACE_TRY(t_resultVariableName, t_expExpression) \
auto ACE_MACRO_CONCAT(_exp, t_resultVariableName) = (t_expExpression); \
if (!ACE_MACRO_CONCAT(_exp, t_resultVariableName)) \
    return {}; \
auto t_resultVariableName = std::move(ACE_MACRO_CONCAT(_exp, t_resultVariableName).Unwrap())

#define ACE_TRY_ASSERT(t_boolExpression) \
if (!(t_boolExpression)) \
    return {}

#define ACE_TRY_UNREACHABLE() \
return {}

#define ACE_TRY_VOID(t_expExpression) \
ACE_TRY_ASSERT(t_expExpression)

namespace Ace
{
    class IError
    {
    public:
    };

    template<typename T>
    class Expected;

    class ExpectedVoidType {};
    inline constexpr const ExpectedVoidType ExpectedVoid{};

    template<>
    class Expected<void>
    {
    public:
        Expected()
            : m_HasValue{ false }
        {
        }

        Expected(const Expected& t_other)
            : m_HasValue{ t_other.m_HasValue }
        {
        }

        auto operator=(const Expected& t_other) -> Expected&
        {
            m_HasValue = t_other.m_HasValue;
            return *this;
        }

        Expected(Expected&& t_other) noexcept
            : m_HasValue{ t_other.m_HasValue }
        {
            t_other.m_HasValue = false;
        }

        auto operator=(Expected&& t_other) noexcept -> Expected&
        {
            m_HasValue = t_other.m_HasValue;

            t_other.m_HasValue = false;

            return *this;
        }

        Expected(const ExpectedVoidType& t_value)
            : m_HasValue(true)
        {
        }

        Expected(ExpectedVoidType&& t_value) noexcept
            : m_HasValue(true)
        {
        }

        ~Expected() = default;

        auto Unwrap() -> void
        {
            if (m_HasValue)
                return;

            ACE_UNREACHABLE();
        }

        operator bool() const
        {
            return m_HasValue;
        }

    private:
        bool m_HasValue;
    };

    template<typename T>
    class Expected
    {
    public:
        Expected()
            : m_HasValue(false)
        {
        }

        template<typename T_ = T, std::enable_if<std::is_copy_constructible_v<T>>* = nullptr>
        Expected(const Expected& t_other)
            : m_HasValue{ t_other.m_HasValue }, m_Value{ t_other.m_Value }
        {
        }

        template<typename T_ = T, std::enable_if<std::is_copy_constructible_v<T>>* = nullptr>
        auto operator=(const Expected& t_other) -> Expected&
        {
            m_HasValue = t_other.m_HasValue;
            m_Value = t_other.m_Value;

            return *this;
        }

        Expected(Expected&& t_other) noexcept
            : m_HasValue{ t_other.m_HasValue }, m_Value{ std::move(t_other.m_Value) }
        {
            t_other.m_HasValue = false;
        }

        auto operator=(Expected&& t_other) noexcept -> Expected&
        {
            m_HasValue = t_other.m_HasValue;

            if (m_HasValue)
            {
                m_Value = std::move(t_other.m_Value);
            }

            t_other.m_HasValue = false;

            return *this;
        }

        template<typename T_ = T, std::enable_if<std::is_copy_constructible_v<T>>* = nullptr>
        Expected(const T& t_value)
            : m_HasValue(true), m_Value(t_value)
        {
        }

        Expected(T&& t_value) noexcept
            : m_HasValue(true), m_Value(std::move(t_value))
        {
        }

        ~Expected() = default;

        auto Unwrap() -> T&
        {
            ACE_ASSERT(m_HasValue);
            return m_Value;
        }

        auto Unwrap() const -> const T&
        {
            ACE_ASSERT(m_HasValue);
            return m_Value;
        }

        operator bool() const
        {
            return m_HasValue;
        }

        template<typename TNew>
        operator Expected<TNew>() const
        {
            ACE_TRY_ASSERT(m_HasValue);
            return Expected<TNew>(std::move(m_Value));
        }

    private:
        bool m_HasValue;
        T m_Value;
    };

#define TIn typename std::decay_t<decltype(*TIt{})>
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap())>

    template<typename TIt, typename F>
    auto TransformExpected(TIt t_begin, TIt t_end, F&& t_func) -> Expected<std::vector<TOut>>
    {
        std::vector<TOut> vec{};
        vec.reserve(std::distance(t_begin, t_end));

        ACE_TRY_ASSERT(std::find_if_not(t_begin, t_end,
        [&](const TIn& t_element)
        {
            auto expOut = t_func(t_element);
            if (!expOut)
                return false;

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
    auto TransformExpectedVector(const std::vector<TIn>& t_inVec, F&& t_func) -> std::enable_if_t<!std::is_same_v<TOut, void>, Expected<std::vector<TOut>>>
    {
        std::vector<TOut> outVec{};
        outVec.reserve(t_inVec.size());

        ACE_TRY_ASSERT(std::find_if_not(begin(t_inVec), end(t_inVec),
        [&](const TIn& t_element)
        {
            auto expOut = t_func(t_element);
            if (!expOut)
                return false;

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
    auto TransformExpectedVector(const std::vector<TIn>& t_inVec, F&& t_func) -> std::enable_if_t<std::is_same_v<TOut, void>, Expected<void>>
    {
        ACE_TRY_ASSERT(std::find_if_not(begin(t_inVec), end(t_inVec),
        [&](const TIn& t_element)
        {
            return t_func(t_element);
        }) == end(t_inVec));

        return ExpectedVoid;
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap())>

    template<typename TIn, typename F>
    auto TransformExpectedOptional(const std::optional<TIn>& t_optIn, F&& t_func) -> Expected<std::optional<TOut>>
    {
        if (!t_optIn.has_value())
            return std::optional<TOut>{};

        ACE_TRY(out, t_func(t_optIn.value()));
        return std::optional{ out };
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap().Value)>

    template<typename TIn, typename F>
    auto TransformExpectedMaybeChangedVector(const std::vector<TIn>& t_inVec, F&& t_func) -> Expected<MaybeChanged<std::vector<TOut>>>
    {
        bool isChanged = false;
        std::vector<TOut> outVec{};
        outVec.reserve(t_inVec.size());

        ACE_TRY_ASSERT(std::find_if_not(begin(t_inVec), end(t_inVec),
        [&](const TIn& t_element)
        {
            auto expMchElement = t_func(t_element);
            if (!expMchElement)
                return false;

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
            return CreateUnchanged(t_inVec);

        return CreateChanged(outVec);
    }

#undef TOut
#define TOut typename std::decay_t<decltype(t_func(TIn{}).Unwrap().Value)>

    template<typename TIn, typename F>
    auto TransformExpectedMaybeChangedOptional(const std::optional<TIn>& t_optIn, F&& t_func) -> Expected<MaybeChanged<std::optional<TOut>>>
    {
        bool isChanged = false;

        if (!t_optIn.has_value())
            return CreateUnchanged(t_optIn);

        ACE_TRY(mchOut, t_func(t_optIn.value()));

        if (!mchOut.IsChanged)
            return CreateUnchanged(t_optIn);

        return CreateChanged(std::optional{ mchOut.Value });
    }

#undef TOut
}
#pragma warning(pop)
