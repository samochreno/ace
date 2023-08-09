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
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        std::string what{ std::string_view{ error.what() }.substr(18) };
        MakeLowercase(what);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file system: " + what + ": " + path.string()
        );

        return group;
    }

    auto CreateFileNotFoundError(
        const std::filesystem::path& path
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file not found: " + path.string()
        );

        return group;
    }

    auto CreateFileOpenError(
        const std::filesystem::path& path
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            std::nullopt,
            "unable to open file: " + path.string()
        );

        return group;
    }

    auto CreateFilePathEndsWithSeparatorError(
        const std::filesystem::path& path
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            std::nullopt,
            "file path ends with separator: " + path.string()
        );

        return group;
    }   
}
