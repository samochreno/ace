#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    auto CreateUnterminatedMultiLineCommentError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnterminatedStringLiteralError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedCharacterError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnknownNumericLiteralTypeSuffixError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;
}
