#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    inline auto CreateUnterminatedMultiLineCommentError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "unterminated multiline comment"
        );
    }

    inline auto CreateUnterminatedStringLiteralError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "unterminated string literal"
        );
    }

    inline auto CreateUnexpectedCharacterError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "unexpected character"
        );
    }

    inline auto CreateUnknownNumericLiteralTypeSuffixError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "unknown numeric literal type suffix"
        );
    }

    inline auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SourceLocation& sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            "decimal point in non-float numeric literal"
        );
    }   
}
