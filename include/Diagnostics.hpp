#pragma once

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "DiagnosticsBase.hpp"
#include "Expected.hpp"
#include "Diagnosed.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    class FileBuffer;

    auto CreateMissingPackagePathArgError() -> std::shared_ptr<const Diagnostic>;
    auto CreateMultiplePackagePathArgsError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateMissingCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUnknownCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateMissingCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUnexpectedCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateJsonError(
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json::exception& t_jsonException
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateFileNotFoundError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateFileOpenError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUnterminatedMultiLineCommentError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUnterminatedStringLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUnexpectedCharacterError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUnknownNumericLiteralTypeSuffixError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
}
