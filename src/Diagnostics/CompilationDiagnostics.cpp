#include "Diagnostics/CompilationDiagnostics.hpp"

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
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

    static auto CreateJsonTypeString(
        const nlohmann::json::value_t type
    ) -> std::string
    {
        switch (type)
        {
            case nlohmann::json::value_t::null:
            {
                return "null";
            }

            case nlohmann::json::value_t::object:
            {
                return "object";
            }

            case nlohmann::json::value_t::array:
            {
                return "array";
            }

            case nlohmann::json::value_t::string:
            {
                return "string";
            }

            case nlohmann::json::value_t::boolean:
            {
                return "boolean";
            }

            case nlohmann::json::value_t::number_integer:
            {
                return "signed integer";
            }

            case nlohmann::json::value_t::number_unsigned:
            {
                return "unsigned integer";
            }

            case nlohmann::json::value_t::number_float:
            {
                return "float";
            }

            case nlohmann::json::value_t::binary:
            {
                return "binary array";
            }

            case nlohmann::json::value_t::discarded:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    static auto CreateJsonTypeStringWithArticle(
        const nlohmann::json::value_t type
    ) -> std::string
    {
        switch (type)
        {
            case nlohmann::json::value_t::null:
            case nlohmann::json::value_t::string:
            case nlohmann::json::value_t::boolean:
            case nlohmann::json::value_t::number_integer:
            case nlohmann::json::value_t::number_float:
            case nlohmann::json::value_t::binary:
            {
                return "a " + CreateJsonTypeString(type);
            }

            case nlohmann::json::value_t::number_unsigned:
            case nlohmann::json::value_t::array:
            case nlohmann::json::value_t::object:
            {
                return "an " + CreateJsonTypeString(type);
            }

            case nlohmann::json::value_t::discarded:
            {
                ACE_UNREACHABLE();
            }
        }
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
