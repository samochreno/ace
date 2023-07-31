#include "Diagnostics/FileSystemDiagnostics.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <filesystem>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "String.hpp"

namespace Ace
{
    auto CreateFileSystemError(
        const std::filesystem::path& path,
        const std::filesystem::filesystem_error& error
    ) -> std::shared_ptr<const Diagnostic>
    {
        std::string what{ std::string_view{ error.what() }.substr(18) };
        MakeLowercase(what);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file system: " + what + ": " + path.string()
        );
    }

    auto CreateFileNotFoundError(
        const std::filesystem::path& path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file not found: " + path.string()
        );
    }

    auto CreateFileOpenError(
        const std::filesystem::path& path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "unable to open file: " + path.string()
        );
    }

    auto CreateFilePathEndsWithSeparatorError(
        const std::filesystem::path& path
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file path ends with separator: " + path.string()
        );
    }   
}
