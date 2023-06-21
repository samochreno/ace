#pragma once

#include <string>
#include <optional>

#include "SourceLocation.hpp"

#define ACE_MACRO_CONCAT_IMPL(t_x, t_y) t_x##t_y
#define ACE_MACRO_CONCAT(t_x, t_y) ACE_MACRO_CONCAT_IMPL(t_x, t_y)

#define ACE_TRY(t_resultVariableName, t_expExpression) \
auto ACE_MACRO_CONCAT(_exp_, t_resultVariableName) = (t_expExpression); \
if (!ACE_MACRO_CONCAT(_exp_, t_resultVariableName)) \
{ \
    return ACE_MACRO_CONCAT(_exp_, t_resultVariableName).GetDiagnosticBag(); \
} \
auto t_resultVariableName = std::move(ACE_MACRO_CONCAT(_exp_, t_resultVariableName).Unwrap())

#define ACE_TRY_ASSERT(t_boolExpression) \
if (!(t_boolExpression)) \
{ \
    return std::make_shared<const NoneError>(); \
}

#define ACE_TRY_UNREACHABLE() \
return std::make_shared<const NoneError>();

#define ACE_TRY_VOID(t_expExpression) \
{ \
    const auto expExpression = (t_expExpression); \
    if (!expExpression) \
    { \
        return expExpression.GetDiagnosticBag(); \
    } \
}

namespace Ace
{
    enum class DiagnosticSeverity
    {
        None,
        Info,
        Warning,
        Error,
    };

    class IDiagnostic
    {
    public:
        virtual ~IDiagnostic() = default;

        virtual auto GetSeverity() const -> DiagnosticSeverity = 0;
        virtual auto GetSourceLocation() const -> std::optional<SourceLocation> = 0;
        virtual auto CreateMessage() const -> std::string = 0;
    };

    class DiagnosticBag;

    class IDiagnosed
    {
    public:
        virtual auto GetDiagnosticBag() const -> const DiagnosticBag& = 0;
    };
}
