#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto CreateMissingPackagePathArgError() -> std::shared_ptr<const Diagnostic>;

    auto CreateMultiplePackagePathArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUnexpectedPackagePropertyWarning(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateMissingPackagePropertyError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUnexpectedPackagePropertyTypeError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName,
        const nlohmann::json::value_t type,
        const nlohmann::json::value_t expectedType
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateUndefinedRefToPackagePathMacroError(
        const FileBuffer* const packageFileBuffer,
        const std::string& macro
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateTrailingPackagePathCharactersBeforeExtensionError(
        const FileBuffer* const packageFileBuffer,
        const std::string_view characters
    ) -> std::shared_ptr<const Diagnostic>;
}
