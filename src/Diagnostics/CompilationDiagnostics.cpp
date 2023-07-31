#include "Diagnostics/CompilationDiagnostics.hpp"

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "DiagnosticStringConversions.hpp"
#include "SrcLocation.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto CreateMissingPackagePathArgError() -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "missing package path argument"
        );
    }

    auto CreateMultiplePackagePathArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "multiple package path arguments"
        );
    }

    auto CreateUnexpectedPackagePropertyWarning(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Warning,
            packageFileBuffer->CreateFirstLocation(),
            "unexpected property `" + propertyName + "`"
        );
    }

    auto CreateMissingPackagePropertyError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            "missing property `" + propertyName + "`"
        );
    }

    auto CreateUnexpectedPackagePropertyTypeError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName,
        const nlohmann::json::value_t type,
        const nlohmann::json::value_t expectedType
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = std::string{} +
            "unexpected `" + CreateJsonTypeString(type) + 
            "` of property `" + propertyName +  "`, expected `" +
            CreateJsonTypeStringWithArticle(expectedType) + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            message
        );
    }

    auto CreateUndefinedRefToPackagePathMacroError(
        const FileBuffer* const packageFileBuffer,
        const std::string& macro
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            "undefined reference to macro `" + macro + "`"
        );
    }

    auto CreateTrailingPackagePathCharactersBeforeExtensionError(
        const FileBuffer* const packageFileBuffer,
        const std::string_view characters
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = std::string{} +
            "trailing characters in path before extension `" +
            std::string{ characters } + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            message
        );
    }
}
