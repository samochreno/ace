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
            "Missing package path argument"
        );
    }

    auto CreateMultiplePackagePathArgsError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Multiple package path arguments"
        );
    }

    auto CreateMissingCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Missing option name"
        );
    }

    auto CreateUnknownCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Unknown option name"
        );
    }

    auto CreateMissingCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Missing option argument"
        );
    }

    auto CreateUnexpectedCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Unexpected option argument"
        );
    }

    auto CreateJsonError(
        const FileBuffer* const t_fileBuffer,
        const nlohmann::json::exception& t_jsonException
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string_view originalMessage = t_jsonException.what();

        const auto closingBracketIt = std::find(
            begin(originalMessage),
            end  (originalMessage),
            ']'
        );
        ACE_ASSERT(closingBracketIt != end(originalMessage));

        auto [message, optSourceLocation] = [&]() -> std::tuple<std::string, std::optional<SourceLocation>>
        {
            const auto atLinePos = originalMessage.find("at line");
            if (atLinePos == std::string::npos)
            {
                const std::string message
                {
                    closingBracketIt + 2,
                    end(originalMessage),
                };

                return { message, std::optional<SourceLocation>{} };
            }

            const auto colonIt = std::find(
                begin(originalMessage),
                end  (originalMessage),
                ':'
            );

            const std::string message
            {
                colonIt + 2,
                end(originalMessage),
            };

            auto it = begin(originalMessage) + atLinePos + 8;

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
        message = "Json error: " + message;

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            optSourceLocation,
            message
        );
    }

    auto CreateFileNotFoundError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "File not found: " + t_path.string()
        );
    }

    auto CreateFileOpenError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "Unable to open file: " + t_path.string()
        );
    }

    auto CreateUnterminatedMultiLineCommentError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Unterminated multiline comment"
        );
    }

    auto CreateUnterminatedStringLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Unterminated string literal"
        );
    }

    auto CreateUnexpectedCharacterError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Unexpected character"
        );
    }

    auto CreateUnknownNumericLiteralTypeSuffixError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Unknown numeric literal type suffix"
        );
    }

    auto CreateDecimalPointInNonFloatNumericLiteralError(
        const SourceLocation& t_sourceLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            t_sourceLocation,
            "Decimal point in non-float numeric literal"
        );
    }
}
