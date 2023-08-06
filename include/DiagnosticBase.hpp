#pragma once

#include <vector>
#include <string>
#include <optional>

#include "SrcLocation.hpp"

#define ACE_MACRO_CONCAIMPL(x, y) x##y
#define ACE_MACRO_CONCAT(x, y) ACE_MACRO_CONCAIMPL(x, y)

#define ACE_TRY(resultVarName, expExpr) \
auto ACE_MACRO_CONCAT(_exp_, resultVarName) = (expExpr); \
if (!ACE_MACRO_CONCAT(_exp_, resultVarName)) \
{ \
    return ACE_MACRO_CONCAT(_exp_, resultVarName).GetDiagnostics(); \
} \
auto resultVarName = std::move(ACE_MACRO_CONCAT(_exp_, resultVarName).Unwrap())

#define ACE_TRY_ASSERT(boolExpr) \
if (!(boolExpr)) \
{ \
    return EmptyError{}; \
}

#define ACE_TRY_UNREACHABLE() \
return EmptyError{};

#define ACE_TRY_VOID(inExpExpr) \
{ \
    const auto expExpr = (inExpExpr); \
    if (!expExpr) \
    { \
        return expExpr.GetDiagnostics(); \
    } \
}

namespace Ace
{
    enum class DiagnosticSeverity
    {
        Info,
        Note,
        Warning,
        Error,
    };

    struct Diagnostic
    {
        DiagnosticSeverity Severity{};
        std::optional<SrcLocation> OptSrcLocation{};
        std::string Message{};
    };

    struct DiagnosticGroup
    {
        std::vector<Diagnostic> Diagnostics{};
    };

    class DiagnosticBag;

    class IDiagnosed
    {
    public:
        virtual auto GetDiagnostics() const -> const DiagnosticBag& = 0;
    };
}
