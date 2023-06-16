#pragma once

#include <optional>

#include "DiagnosticsBase.hpp"
#include "Expected.hpp"
#include "Diagnosed.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    class ISourceDiagnostic : public virtual IDiagnostic
    {
    public:
        virtual ~ISourceDiagnostic() = default;

        virtual auto GetSourceLocation() const -> const SourceLocation& = 0;
    };

    class UnterminatedMultiLineCommentError : public virtual ISourceDiagnostic
    {
    public:
        UnterminatedMultiLineCommentError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation }
        {
        }
        virtual ~UnterminatedMultiLineCommentError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetMessage() const -> const char* final
        {
            return "Unterminated multiline comment";
        }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };
    
    class UnterminatedStringLiteralError : public virtual ISourceDiagnostic
    {
    public:
        UnterminatedStringLiteralError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation }
        {
        }
        virtual ~UnterminatedStringLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetMessage() const -> const char* final
        {
            return "Unterminated string literal";
        }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnexpectedCharacterError : public virtual ISourceDiagnostic
    {
    public:
        UnexpectedCharacterError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~UnexpectedCharacterError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetMessage() const -> const char* final
        {
            return "Unexpected character";
        }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };

    class InvalidNumericLiteralTypeSuffixError : public virtual ISourceDiagnostic
    {
    public:
        InvalidNumericLiteralTypeSuffixError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~InvalidNumericLiteralTypeSuffixError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetMessage() const -> const char* final
        {
            return "Invalid numeric literal type suffix";
        }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };

    class DecimalPointInNonFloatNumericLiteralError : public virtual ISourceDiagnostic
    {
    public:
        DecimalPointInNonFloatNumericLiteralError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~DecimalPointInNonFloatNumericLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetMessage() const -> const char* final
        {
            return "Decimal point in non-float numeric literal";
        }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };
}
