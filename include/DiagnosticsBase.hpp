#pragma once

#include <string>
#include <optional>

#include "SourceLocation.hpp"

#define ACE_MACRO_CONCAT_IMPL(t_x, t_y) t_x##t_y
#define ACE_MACRO_CONCAT(t_x, t_y) ACE_MACRO_CONCAT_IMPL(t_x, t_y)

#define ACE_TRY(t_resultVarName, t_expExpr) \
auto ACE_MACRO_CONCAT(_exp_, t_resultVarName) = (t_expExpr); \
if (!ACE_MACRO_CONCAT(_exp_, t_resultVarName)) \
{ \
    return ACE_MACRO_CONCAT(_exp_, t_resultVarName).GetDiagnosticBag(); \
} \
auto t_resultVarName = std::move(ACE_MACRO_CONCAT(_exp_, t_resultVarName).Unwrap())

#define ACE_TRY_ASSERT(t_boolExpr) \
if (!(t_boolExpr)) \
{ \
    return std::make_shared<const NoneError>(); \
}

#define ACE_TRY_UNREACHABLE() \
return std::make_shared<const NoneError>();

#define ACE_TRY_VOID(t_expExpr) \
{ \
    const auto expExpr = (t_expExpr); \
    if (!expExpr) \
    { \
        return expExpr.GetDiagnosticBag(); \
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
