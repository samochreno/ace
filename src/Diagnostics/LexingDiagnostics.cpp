#include "Diagnostics/LexingDiagnostics.hpp"

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateUnterminatedMultiLineCommentError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unterminated multiline comment"
        );

        return group;
    }

    auto CreateUnterminatedStringLiteralError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unterminated string literal"
        );

        return group;
    }

    auto CreateUnexpectedCharacterError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unexpected character"
        );

        return group;
    }

    auto CreateUnknownNumericLiteralTypeSuffixError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unknown numeric literal type suffix"
        );

        return group;
    }

    auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "decimal point in non-float numeric literal"
        );

        return group;
    }   
}
