#include "Diagnostics.hpp"

#include <string>
#include <optional>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "SourceLocation.hpp"
#include "FileBuffer.hpp"
#include "Utility.hpp"

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
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "multiple package path arguments"
        );
    }

    auto CreateMissingCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "missing option name"
        );
    }

    auto CreateUnknownCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unknown option name"
        );
    }

    auto CreateMissingCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "missing option argument"
        );
    }

    auto CreateUnexpectedCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unexpected option argument"
        );
    }

    auto CreateJsonError(
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json::exception& t_jsonException
    ) -> std::shared_ptr<const Diagnostic>
    {
        std::string what = t_jsonException.what();
        MakeLowercase(what);

        const auto closingBracketIt = std::find(
            begin(what),
            end  (what),
            ']'
        );
        ACE_ASSERT(closingBracketIt != end(what));

        auto [message, sourceLocation] = [&]() -> std::tuple<std::string, std::optional<SourceLocation>>
        {
            const auto atLinePos = what.find("at line");
            if (atLinePos == std::string::npos)
            {
                const std::string message
                {
                    closingBracketIt + 2,
                    end(what),
                };

                return { message, t_fileBuffer->CreateFirstLocation() };
            }

            const auto colonIt = std::find(
                begin(what),
                end  (what),
                ':'
            );

            const std::string message
            {
                colonIt + 2,
                end(what),
            };

            auto it = begin(what) + atLinePos + 8;

            std::string lineString{};
            while (IsNumber(*it))
            {
                lineString += *it;
                ++it;
            }

            it += 9;

            std::string columnString{};
            while (IsNumber(*it))
            {
                columnString += *it;
                ++it;
            }

            const auto lineIndex =
                static_cast<size_t>(std::stoi(lineString) - 1);

            const auto characterIndex =
                static_cast<size_t>(std::stoi(columnString));

            const auto& line = t_fileBuffer->GetLines().at(lineIndex);
            const SourceLocation sourceLocation
            {
                t_fileBuffer,
                begin(line) + characterIndex,
                begin(line) + characterIndex + 1,
            };

            return
            {
                message,
                sourceLocation,
            };
        }();

        message.at(0) = std::toupper(message.at(0));
        message = "json: " + message;

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            sourceLocation,
            message
        );
    }

    auto CreateUnexpectedPackagePropertyWarning(
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

    auto CreateMissingPackagePropertyError(
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

    static auto CreateJsonTypeString(
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

    auto CreateUnexpectedPackagePropertyTypeError(
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

    auto CreateUndefinedReferenceToPackagePathMacroError(
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

    auto CreateFileSystemError(
        const std::filesystem::path& t_path,
        const std::filesystem::filesystem_error& t_error
    ) -> std::shared_ptr<const Diagnostic>
    {
        std::string what{ std::string_view{ t_error.what() }.substr(18) };
        MakeLowercase(what);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file system: " + what + ": " + t_path.string()
        );
    }

    auto CreateFileNotFoundError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file not found: " + t_path.string()
        );
    }

    auto CreateFileOpenError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "unable to open file: " + t_path.string()
        );
    }

    auto CreateFilePathEndsWithSeparatorError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file path ends with separator: " + t_path.string()
        );
    }

    auto CreateUnterminatedMultiLineCommentError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unterminated multiline comment"
        );
    }

    auto CreateUnterminatedStringLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unterminated string literal"
        );
    }

    auto CreateUnexpectedCharacterError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unexpected character"
        );
    }

    auto CreateUnknownNumericLiteralTypeSuffixError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "unknown numeric literal type suffix"
        );
    }

    auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "decimal point in non-float numeric literal"
        );
    }
}
