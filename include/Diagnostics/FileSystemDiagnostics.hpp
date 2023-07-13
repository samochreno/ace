#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <filesystem>

#include "Diagnostic.hpp"
#include "SourceLocation.hpp"
#include "String.hpp"

namespace Ace
{
    inline auto CreateFileSystemError(
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

    inline auto CreateFileNotFoundError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file not found: " + t_path.string()
        );
    }

    inline auto CreateFileOpenError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "unable to open file: " + t_path.string()
        );
    }

    inline auto CreateFilePathEndsWithSeparatorError(
        const std::filesystem::path& t_path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file path ends with separator: " + t_path.string()
        );
    }   
}
