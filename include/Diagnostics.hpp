#pragma once

#include "DiagnosticsBase.hpp"
#include "Expected.hpp"
#include "Diagnosed.hpp"

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
        UnclosedMultiLineCommentError() = default;
        virtual ~UnclosedMultiLineCommentError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }
    };

    class ILexerScanDiagnostic : public virtual ILexerDiagnostic
    {
    public:
        virtual ~ILexerScanDiagnostic() = default;

        virtual auto GetOffset() const -> size_t = 0;
        virtual auto GetLength() const -> size_t = 0;
    };
    
    class UnclosedStringLiteralError : public virtual ILexerScanDiagnostic
    {
    public:
        UnclosedStringLiteralError(
            const size_t& t_offset
        ) : m_Offset{ t_offset }
        {
        }
        virtual ~UnclosedStringLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }

        auto GetOffset() const -> size_t final { return m_Offset; }
        auto GetLength() const -> size_t final { return 1; }

    private:
        size_t m_Offset{};
    };

    class UnexpectedCharacterError : public virtual ILexerScanDiagnostic
    {
    public:
        UnexpectedCharacterError() = default;
        virtual ~UnexpectedCharacterError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }

        auto GetOffset() const -> size_t final { return 0; }
        auto GetLength() const -> size_t final { return 1; }
    };

    class InvalidNumericLiteralTypeSuffix : public virtual ILexerScanDiagnostic
    {
    public:
        InvalidNumericLiteralTypeSuffix(
            const size_t& t_offset,
            const size_t& t_length
        ) : m_Length{ t_length }
        {
        }
        virtual ~InvalidNumericLiteralTypeSuffix() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }

        auto GetOffset() const -> size_t final { return m_Offset; }
        auto GetLength() const -> size_t final { return m_Length; }

    private:
        size_t m_Offset{};
        size_t m_Length{};
    };

    class MultipleDecimalPointsInNumericLiteralError : public virtual ILexerScanDiagnostic
    {
    public:
        MultipleDecimalPointsInNumericLiteralError(
            const size_t& t_offset
        ) : m_Offset{ t_offset }
        {
        }
        virtual ~MultipleDecimalPointsInNumericLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final { return DiagnosticSeverity::Error; }

        auto GetOffset() const -> size_t final { return m_Offset; }
        auto GetLength() const -> size_t final { return 1; }

    private:
        size_t m_Offset{};
    };
}
