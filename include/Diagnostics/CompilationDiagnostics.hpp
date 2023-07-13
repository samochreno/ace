#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "SourceLocation.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    inline auto CreateMissingPackagePathArgError() -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "missing package path argument"
        );
    }

    inline auto CreateMultiplePackagePathArgsError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "multiple package path arguments"
        );
    }

    inline auto CreateUnexpectedPackagePropertyWarning(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_propertyName
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Warning,
            t_packageFileBuffer->CreateFirstLocation(),
            "unexpected property `" + t_propertyName + "`"
        );
    }

    inline auto CreateMissingPackagePropertyError(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_propertyName
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_packageFileBuffer->CreateFirstLocation(),
            "missing property `" + t_propertyName + "`"
        );
    }

    inline auto CreateJsonTypeString(
        const nlohmann::json::value_t t_type
    ) -> const char* 
    {
        switch (t_type)
        {
            case nlohmann::json::value_t::null:            return "null";
            case nlohmann::json::value_t::object:          return "object";
            case nlohmann::json::value_t::array:           return "array";
            case nlohmann::json::value_t::string:          return "string";
            case nlohmann::json::value_t::boolean:         return "boolean";
            case nlohmann::json::value_t::number_integer:  return "signed integer";
            case nlohmann::json::value_t::number_unsigned: return "unsigned integer";
            case nlohmann::json::value_t::number_float:    return "float";
            case nlohmann::json::value_t::binary:          return "binary array";

            case nlohmann::json::value_t::discarded:       ACE_UNREACHABLE();
        }
    }

    inline auto CreateUnexpectedPackagePropertyTypeError(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_propertyName,
        const nlohmann::json::value_t t_type,
        const nlohmann::json::value_t t_expectedType
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = std::string{} +
            "unexpected type `" + CreateJsonTypeString(t_type) + 
            "` of property `" + t_propertyName +  "`, expected `" +
            CreateJsonTypeString(t_expectedType) + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_packageFileBuffer->CreateFirstLocation(),
            message
        );
    }

    inline auto CreateUndefinedReferenceToPackagePathMacroError(
        const FileBuffer* const t_packageFileBuffer,
        const std::string& t_macro
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_packageFileBuffer->CreateFirstLocation(),
            "undefined reference to macro `" + t_macro + "`"
        );
    }   
}
