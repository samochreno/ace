#pragma once

#include <string>
#include <optional>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "DiagnosticsBase.hpp"
#include "Expected.hpp"
#include "Diagnosed.hpp"
#include "SourceLocation.hpp"

namespace Ace
{
    class MissingPackagePathArgError : public virtual IDiagnostic
    {
    public:
        MissingPackagePathArgError() = default;
        virtual ~MissingPackagePathArgError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;
    };

    class MultiplePackagePathArgsError : public virtual IDiagnostic
    {
    public:
        MultiplePackagePathArgsError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~MultiplePackagePathArgsError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class MissingCommandLineOptionNameError : public virtual IDiagnostic
    {
    public:
        MissingCommandLineOptionNameError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~MissingCommandLineOptionNameError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnknownCommandLineOptionNameError : public virtual IDiagnostic
    {
    public:
        UnknownCommandLineOptionNameError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~UnknownCommandLineOptionNameError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class MissingCommandLineOptionValueError : public virtual IDiagnostic
    {
    public:
        MissingCommandLineOptionValueError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~MissingCommandLineOptionValueError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnexpectedCommandLineOptionValueError : public virtual IDiagnostic
    {
    public:
        UnexpectedCommandLineOptionValueError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~UnexpectedCommandLineOptionValueError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class JsonError : public virtual IDiagnostic
    {
    public:
        JsonError(const nlohmann::json::exception& t_jsonException);
        virtual ~JsonError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        const char* m_Message{};
    };

    class FileNotFoundError : public virtual IDiagnostic
    {
    public:
        FileNotFoundError(const std::filesystem::path& t_path);
        virtual ~FileNotFoundError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        std::filesystem::path m_Path{};
    };

    class FileOpenError : public virtual IDiagnostic
    {
    public:
        FileOpenError(const std::filesystem::path& t_path);
        virtual ~FileOpenError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        std::filesystem::path m_Path{};
    };

    class UnterminatedMultiLineCommentError : public virtual IDiagnostic
    {
    public:
        UnterminatedMultiLineCommentError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~UnterminatedMultiLineCommentError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };
    
    class UnterminatedStringLiteralError : public virtual IDiagnostic
    {
    public:
        UnterminatedStringLiteralError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~UnterminatedStringLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnexpectedCharacterError : public virtual IDiagnostic
    {
    public:
        UnexpectedCharacterError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~UnexpectedCharacterError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class UnknownNumericLiteralTypeSuffixError : public virtual IDiagnostic
    {
    public:
        UnknownNumericLiteralTypeSuffixError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~UnknownNumericLiteralTypeSuffixError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };

    class DecimalPointInNonFloatNumericLiteralError : public virtual IDiagnostic
    {
    public:
        DecimalPointInNonFloatNumericLiteralError(
            const SourceLocation& t_sourceLocation
        );
        virtual ~DecimalPointInNonFloatNumericLiteralError() = default;

        auto GetSeverity() const -> DiagnosticSeverity final;
        auto GetSourceLocation() const -> std::optional<SourceLocation> final;
        auto CreateMessage() const -> std::string final;

    private:
        SourceLocation m_SourceLocation{};
    };
}
