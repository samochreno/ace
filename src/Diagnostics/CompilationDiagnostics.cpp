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
    auto CreateMissingPackagePathArgError() -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            std::nullopt,
            "missing package path argument"
        );

        return group;
    }

    auto CreateMultiplePackagePathArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "multiple package path arguments"
        );

        return group;
    }

    auto CreateUnexpectedPackagePropertyWarning(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Warning,
            packageFileBuffer->CreateFirstLocation(),
            "unexpected property `" + propertyName + "`"
        );

        return group;
    }

    auto CreateMissingPackagePropertyError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            "missing property `" + propertyName + "`"
        );

        return group;
    }

    auto CreateUnexpectedPackagePropertyTypeError(
        const FileBuffer* const packageFileBuffer,
        const std::string& propertyName,
        const nlohmann::json::value_t type,
        const nlohmann::json::value_t expectedType
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message = std::string{} +
            "unexpected `" + CreateJsonTypeString(type) + 
            "` of property `" + propertyName +  "`, expected `" +
            CreateJsonTypeStringWithArticle(expectedType) + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            message
        );

        return group;
    }

    auto CreateUndefinedRefToPackagePathMacroError(
        const FileBuffer* const packageFileBuffer,
        const std::string& macro
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            "undefined reference to macro `" + macro + "`"
        );

        return group;
    }

    auto CreateTrailingPackagePathCharactersBeforeExtensionError(
        const FileBuffer* const packageFileBuffer,
        const std::string_view characters
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message = std::string{} +
            "trailing characters in path before extension `" +
            std::string{ characters } + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            packageFileBuffer->CreateFirstLocation(),
            message
        );

        return group;
    }
}
