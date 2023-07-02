#include "Diagnostics.hpp"

namespace Ace
{
    auto MissingPackagePathArgError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto MissingPackagePathArgError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return std::nullopt;
    }

    auto MissingPackagePathArgError::CreateMessage() const -> std::string
    {
        return "Missing package path argument";
    }

    MultiplePackagePathArgsError::MultiplePackagePathArgsError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation }
    {
    }

    auto MultiplePackagePathArgsError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto MultiplePackagePathArgsError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto MultiplePackagePathArgsError::CreateMessage() const -> std::string
    {
        return "Multiple package path arguments";
    }

    MissingCommandLineOptionNameError::MissingCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation }
    {
    }

    auto MissingCommandLineOptionNameError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto MissingCommandLineOptionNameError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto MissingCommandLineOptionNameError::CreateMessage() const -> std::string
    {
        return "Missing option name";
    }

    UnknownCommandLineOptionNameError::UnknownCommandLineOptionNameError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation }
    {
    }

    auto UnknownCommandLineOptionNameError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto UnknownCommandLineOptionNameError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto UnknownCommandLineOptionNameError::CreateMessage() const -> std::string
    {
        return "Unknown option name";
    }

    MissingCommandLineOptionValueError::MissingCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation }
    {
    }

    auto MissingCommandLineOptionValueError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto MissingCommandLineOptionValueError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto MissingCommandLineOptionValueError::CreateMessage() const -> std::string
    {
        return "Missing option arg";
    }

    UnexpectedCommandLineOptionValueError::UnexpectedCommandLineOptionValueError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation }
    {
    }

    auto UnexpectedCommandLineOptionValueError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto UnexpectedCommandLineOptionValueError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto UnexpectedCommandLineOptionValueError::CreateMessage() const -> std::string
    {
        return "Unexpected option argumet";
    }

    JsonError::JsonError(
        const nlohmann::json::exception& t_jsonException
    ) : m_Message{ t_jsonException.what() }
    {
    }

    auto JsonError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto JsonError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return std::nullopt;
    }

    auto JsonError::CreateMessage() const -> std::string
    {
        return "Unexpected option argument";
    }

    FileNotFoundError::FileNotFoundError(
        const std::filesystem::path& t_path
    ) : m_Path{ t_path }
    {
    }

    auto FileNotFoundError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto FileNotFoundError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return std::nullopt;
    }

    auto FileNotFoundError::CreateMessage() const -> std::string
    {
        return "File not found: " + m_Path.string();
    }

    FileOpenError::FileOpenError(
        const std::filesystem::path& t_path
    ) : m_Path{ t_path }
    {
    }

    auto FileOpenError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto FileOpenError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return std::nullopt;
    }

    auto FileOpenError::CreateMessage() const -> std::string
    {
        return "Unable to open file: " + m_Path.string();
    }

    UnterminatedMultiLineCommentError::UnterminatedMultiLineCommentError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation }
    {
    }

    auto UnterminatedMultiLineCommentError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto UnterminatedMultiLineCommentError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto UnterminatedMultiLineCommentError::CreateMessage() const -> std::string
    {
        return "Unterminated multiline comment";
    }

    UnterminatedStringLiteralError::UnterminatedStringLiteralError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation }
    {
    }

    auto UnterminatedStringLiteralError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto UnterminatedStringLiteralError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto UnterminatedStringLiteralError::CreateMessage() const -> std::string
    {
        return "Unterminated string literal";
    }

    UnexpectedCharacterError::UnexpectedCharacterError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation } 
    {
    }

    auto UnexpectedCharacterError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto UnexpectedCharacterError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto UnexpectedCharacterError::CreateMessage() const -> std::string
    {
        return "Unexpected character";
    }

    UnknownNumericLiteralTypeSuffixError::UnknownNumericLiteralTypeSuffixError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation } 
    {
    }

    auto UnknownNumericLiteralTypeSuffixError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto UnknownNumericLiteralTypeSuffixError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto UnknownNumericLiteralTypeSuffixError::CreateMessage() const -> std::string
    {
        return "Unknown numeric literal type suffix";
    }

    DecimalPointInNonFloatNumericLiteralError::DecimalPointInNonFloatNumericLiteralError(
        const SourceLocation& t_sourceLocation
    ) : m_SourceLocation{ t_sourceLocation } 
    {
    }

    auto DecimalPointInNonFloatNumericLiteralError::GetSeverity() const -> DiagnosticSeverity
    {
        return DiagnosticSeverity::Error;
    }

    auto DecimalPointInNonFloatNumericLiteralError::GetSourceLocation() const -> std::optional<SourceLocation>
    {
        return m_SourceLocation;
    }

    auto DecimalPointInNonFloatNumericLiteralError::CreateMessage() const -> std::string
    {
        return "Decimal point in non-float numeric literal";
    }
}
