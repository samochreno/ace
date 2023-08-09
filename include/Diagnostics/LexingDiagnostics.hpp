#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateUnterminatedMultiLineCommentError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnterminatedStringLiteralError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnexpectedCharacterError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnknownNumericLiteralTypeSuffixError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;
}
