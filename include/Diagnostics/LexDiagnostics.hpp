#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    inline auto CreateUnterminatedMultiLineCommentError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unterminated multiline comment"
        );
    }

    inline auto CreateUnterminatedStringLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unterminated string literal"
        );
    }

    inline auto CreateUnexpectedCharacterError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unexpected character"
        );
    }

    inline auto CreateUnknownNumericLiteralTypeSuffixError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unknown numeric literal type suffix"
        );
    }

    inline auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "decimal point in non-float numeric literal"
        );
    }   
}
