#pragma once

#include <string>
#include <optional>
#include <filesystem>

#include "DiagnosticsBase.hpp"
#include "Expected.hpp"
#include "Diagnosed.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    class FileNotFoundError : public virtual IDiagnostic
    {
    public:
        FileNotFoundError(
            const std::filesystem::path& t_path
        ) : m_Path{ t_path }
        {
        }
        virtual ~FileNotFoundError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final
        {
            return DiagnosticSeverity::Error;
        }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final
        {
            return std::nullopt;
        }
        auto CreateMessage() const -> std::string final
        {
            return "File not found: " + m_Path.string();
        }

    private:
        std::filesystem::path m_Path{};
    };

    class FileOpenError : public virtual IDiagnostic
    {
    public:
        FileOpenError(
            const std::filesystem::path& t_path
        ) : m_Path{ t_path }
        {
        }

        auto GetSeverity() const -> DiagnosticSeverity final
        {
            return DiagnosticSeverity::Error;
        }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final
        {
            return std::nullopt;
        }
        auto CreateMessage() const -> std::string final
        {
            return "Unable to open file: " + m_Path.string();
        }

    private:
        std::filesystem::path m_Path{};
    };

    class UnterminatedMultiLineCommentError : public virtual IDiagnostic
    {
    public:
        UnterminatedMultiLineCommentError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation }
        {
        }
        virtual ~UnterminatedMultiLineCommentError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final
        {
            return DiagnosticSeverity::Error;
        }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final
        {
            return m_SourceLocation;
        }
        auto CreateMessage() const -> std::string final
        {
            return "Unterminated multiline comment";
        }

    private:
        SourceLocation m_SourceLocation{};
    };
    
    class UnterminatedStringLiteralError : public virtual IDiagnostic
    {
    public:
        UnterminatedStringLiteralError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation }
        {
        }
        virtual ~UnterminatedStringLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final
        {
            return DiagnosticSeverity::Error;
        }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final
        {
            return m_SourceLocation;
        }
        auto CreateMessage() const -> std::string final
        {
            return "Unterminated string literal";
        }

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnexpectedCharacterError : public virtual IDiagnostic
    {
    public:
        UnexpectedCharacterError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~UnexpectedCharacterError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final
        {
            return DiagnosticSeverity::Error;
        }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final
        {
            return m_SourceLocation;
        }
        auto CreateMessage() const -> std::string final
        {
            return "Unexpected character";
        }

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnknownNumericLiteralTypeSuffixError : public virtual IDiagnostic
    {
    public:
        UnknownNumericLiteralTypeSuffixError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~UnknownNumericLiteralTypeSuffixError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final
        {
            return DiagnosticSeverity::Error;
        }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final
        {
            return m_SourceLocation;
        }
        auto CreateMessage() const -> std::string final
        {
            return "Unknown numeric literal type suffix";
        }

    private:
        SourceLocation m_SourceLocation{};
    };

    class DecimalPointInNonFloatNumericLiteralError : public virtual IDiagnostic
    {
    public:
        DecimalPointInNonFloatNumericLiteralError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~DecimalPointInNonFloatNumericLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final
        {
            return DiagnosticSeverity::Error;
        }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final
        {
            return m_SourceLocation;
        }
        auto CreateMessage() const -> std::string final
        {
            return "Decimal point in non-float numeric literal";
        }

    private:
        SourceLocation m_SourceLocation{};
    };
}
