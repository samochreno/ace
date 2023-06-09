#pragma once

#include <optional>

#include "DiagnosticsBase.hpp"
#include "Expected.hpp"
#include "Diagnosed.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    class ILexerDiagnostic : public virtual IDiagnostic
    {
    public:
        virtual ~ILexerDiagnostic() = default;
    };

    class UnclosedMultiLineCommentError : public virtual ILexerDiagnostic
    {
    public:
        UnclosedMultiLineCommentError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation }
        {
        }
        virtual ~UnclosedMultiLineCommentError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };
    
    class UnclosedStringLiteralError : public virtual ILexerDiagnostic
    {
    public:
        UnclosedStringLiteralError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation }
        {
        }
        virtual ~UnclosedStringLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnexpectedCharacterError : public virtual ILexerDiagnostic
    {
    public:
        UnexpectedCharacterError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~UnexpectedCharacterError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };

    class InvalidNumericLiteralTypeSuffixError : public virtual ILexerDiagnostic
    {
    public:
        InvalidNumericLiteralTypeSuffixError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~InvalidNumericLiteralTypeSuffixError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };

    class DecimalPointInNonFloatNumericLiteralError : public virtual ILexerDiagnostic
    {
    public:
        DecimalPointInNonFloatNumericLiteralError(
            const SourceLocation& t_sourceLocation
        ) : m_SourceLocation{ t_sourceLocation } 
        {
        }
        virtual ~DecimalPointInNonFloatNumericLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
        auto GetSourceLocation() const -> std::optional<SourceLocation> final { return m_SourceLocation; }

    private:
        SourceLocation m_SourceLocation{};
    };
}
