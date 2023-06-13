#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <type_traits>
#include <algorithm>

#include "DiagnosticsBase.hpp"
#include "SourceLocation.hpp"
#include "Asserts.hpp"
#include "MaybeChanged.hpp"

#define ACE_MACRO_CONCAT_IMPL(t_x, t_y) t_x##t_y
#define ACE_MACRO_CONCAT(t_x, t_y) ACE_MACRO_CONCAT_IMPL(t_x, t_y)

#define ACE_TRY(t_resultVariableName, t_expExpression) \
auto ACE_MACRO_CONCAT(_exp, t_resultVariableName) = (t_expExpression); \
if (!ACE_MACRO_CONCAT(_exp, t_resultVariableName)) \
    return ACE_MACRO_CONCAT(_exp, t_resultVariableName).GetError(); \
auto t_resultVariableName = std::move(ACE_MACRO_CONCAT(_exp, t_resultVariableName).Unwrap())

#define ACE_TRY_ASSERT(t_boolExpression) \
if (!(t_boolExpression)) \
    return std::make_shared<const NoneError>();

#define ACE_TRY_UNREACHABLE() \
return std::make_shared<const NoneError>();

#define ACE_TRY_VOID(t_expExpression) \
{ \
    const auto expExpression = (t_expExpression); \
    if (!expExpression) \
    { \
        return expExpression.GetError(); \
    } \
}

namespace Ace
{
    class NoneError : public virtual IDiagnostic
    {
    public:
        virtual ~NoneError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final { return std::nullopt; }
        auto GetMessage() const -> const char*
        {
            return "Empty error";
        }
    };

    template<typename TValue, typename TError = NoneError>
    class Expected;

    class ExpectedVoidType {};
    inline constexpr const ExpectedVoidType ExpectedVoid{};

    template<typename TError>
    class Expected<void, TError>
    {
        static_assert(
            std::is_base_of_v<IDiagnostic, TError>,
            "TError must be derived from IDiagnostic"
        );

    public:
        Expected()
        {
        }
        Expected(const Expected& t_other)
            : m_OptError{ t_other.m_OptError }
        {
        }
        Expected(Expected&& t_other) noexcept
            : m_OptError{ std::move(t_other.m_OptError) }
        {
        }
        Expected(const ExpectedVoidType& t_value)
        {
        }
        template<
            typename TErrorNew,
            typename = std::enable_if_t<std::is_base_of_v<TError, TErrorNew>>
        >
        Expected(const std::shared_ptr<const TErrorNew>& t_error)
            : m_OptError{ t_error }
        {
        }
        ~Expected() = default;

        auto operator=(const Expected& t_other) -> Expected&
        {
            m_OptError = t_other.m_OptError;

            return *this;
        }
        auto operator=(Expected&& t_other) noexcept -> Expected&
        {
            m_OptError = std::move(t_other.m_OptError);

            return *this;
        }

        operator bool() const
        {
            return !m_OptError.has_value();
        }

        auto Unwrap() -> void
        {
            if (!m_OptError.has_value())
                return;

            ACE_UNREACHABLE();
        }

        auto GetError() const -> const std::shared_ptr<const TError>&
        {
            return m_OptError.value();
        }

    private:
        std::optional<std::shared_ptr<const TError>> m_OptError{};
    };

    template<typename TValue, typename TError>
    class Expected
    {
        static_assert(
            std::is_base_of_v<IDiagnostic, TError>,
            "TError must be derived from IDiagnostic"
        );

    public:
        Expected()
        {
        }
        Expected(const Expected& t_other)
            : m_OptValue{ t_other.m_OptValue }, m_OptError{ t_other.m_OptError }
        {
        }
        Expected(Expected&& t_other) noexcept
            : m_OptValue{ std::move(t_other.m_OptValue) }, m_OptError{ std::move(t_other.m_OptError) }
        {
        }
        Expected(const TValue& t_value)
            : m_OptValue{ t_value }
        {
        }
        Expected(TValue&& t_value) noexcept
            : m_OptValue{ std::move(t_value) }
        {
        }
        template<typename TErrorNew, typename = std::enable_if_t<std::is_base_of_v<TError, TErrorNew>>>
        Expected(const std::shared_ptr<const TErrorNew>& t_error)
            : m_OptError{ t_error }
        {
        }
        ~Expected() = default;

        auto operator=(const Expected& t_other) -> Expected&
        {
            m_OptValue = t_other.m_OptValue;
            m_OptError = t_other.m_OptError;

            return *this;
        }
        auto operator=(Expected&& t_other) noexcept -> Expected&
        {
            m_OptValue = std::move(t_other.m_OptValue);
            m_OptError = std::move(t_other.m_OptError);

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

        auto GetError() const -> const std::shared_ptr<const TError>&
        {
            return m_OptError.value();
        }

        template<typename TValueNew, typename TErrorNew>
        operator Expected<TValueNew, TErrorNew>() const
        {
            if (*this)
            {
                return Expected<TValueNew, TErrorNew>(m_OptValue.value());
            }
            else
            {
                return Expected<TValueNew, TErrorNew>(m_OptError.value());
            }
        }

    private:
        std::optional<TValue> m_OptValue{};
        std::optional<std::shared_ptr<const TError>> m_OptError{};
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

        return ExpectedVoid;
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
            return std::optional<TOut>{};

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
    auto TransformExpectedMaybeChangedOptional(
        const std::optional<TIn>& t_optIn,
        F&& t_func
    ) -> Expected<MaybeChanged<std::optional<TOut>>>
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
