#pragma once

#include <optional>

#include "DiagnosticsBase.hpp"
#include "Expected.hpp"
#include "Diagnosed.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    class UnterminatedMultiLineCommentError : public virtual IDiagnostic
    {
    public:
        UnterminatedMultiLineCommentError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation }
        {
        }
        virtual ~UnterminatedMultiLineCommentError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }
        auto GetMessage() const -> const char* final
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

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }
        auto GetMessage() const -> const char* final
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

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }
        auto GetMessage() const -> const char* final
        {
            return "Unexpected character";
        }

    private:
        SourceLocation m_SourceLocation{};
    };

    class InvalidNumericLiteralTypeSuffixError : public virtual IDiagnostic
    {
    public:
        InvalidNumericLiteralTypeSuffixError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~InvalidNumericLiteralTypeSuffixError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }
        auto GetMessage() const -> const char* final
        {
            return "Invalid numeric literal type suffix";
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

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> const SourceLocation& final { return m_SourceLocation; }
        auto GetMessage() const -> const char* final
        {
            return "Decimal point in non-float numeric literal";
        }

    private:
        SourceLocation m_SourceLocation{};
    };
}
