#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto CreateMissingPackagePathArgError() -> DiagnosticGroup;

    auto CreateMultiplePackagePathArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnexpectedPackagePropertyWarning(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> DiagnosticGroup;

    auto CreateMissingPackagePropertyError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> DiagnosticGroup;

    auto CreateUnexpectedPackagePropertyTypeError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName,
        const nlohmann::json::value_t type,
        const nlohmann::json::value_t expectedType
    ) -> DiagnosticGroup;

    auto CreateUndefinedRefToPackagePathMacroError(
        const FileBuffer* const packageFileBuffer,
        const std::string& macro
    ) -> DiagnosticGroup;

    auto CreateTrailingPackagePathCharactersBeforeExtensionError(
        const FileBuffer* const packageFileBuffer,
        const std::string_view characters
    ) -> DiagnosticGroup;
}
