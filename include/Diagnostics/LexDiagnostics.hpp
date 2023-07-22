#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    inline auto CreateUnterminatedMultiLineCommentError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unterminated multiline comment"
        );
    }

    inline auto CreateUnterminatedStringLiteralError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unterminated string literal"
        );
    }

    inline auto CreateUnexpectedCharacterError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unexpected character"
        );
    }

    inline auto CreateUnknownNumericLiteralTypeSuffixError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "unknown numeric literal type suffix"
        );
    }

    inline auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "decimal point in non-float numeric literal"
        );
    }   
}
