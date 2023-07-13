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

    auto CreateMissingPackagePathArgError() -> std::shared_ptr<const Diagnostic>;
    auto CreateMultiplePackagePathArgsError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUnexpectedPackagePropertyWarning(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_propertyName
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateMissingPackagePropertyError(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_propertyName
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUnexpectedPackagePropertyTypeError(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_propertyName,
        const nlohmann::json::value_t t_type,
        const nlohmann::json::value_t t_expectedType
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateUndefinedReferenceToPackagePathMacroError(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_macro
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateFileSystemError(
        const std::filesystem::path& t_path,
        const std::filesystem::filesystem_error& t_error
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateFileNotFoundError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateFileOpenError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>;
    auto CreateFilePathEndsWithSeparatorError(
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
