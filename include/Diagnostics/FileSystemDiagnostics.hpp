#pragma once

#include <memory>
#include <filesystem>

#include "Diagnostic.hpp"

namespace Ace
{
    auto CreateFileSystemError(
        const std::filesystem::path& path,
        const std::filesystem::filesystem_error& error
    ) -> DiagnosticGroup;

    auto CreateFileNotFoundError(
        const std::filesystem::path& path
    ) -> DiagnosticGroup;

    auto CreateFileOpenError(
        const std::filesystem::path& path
    ) -> DiagnosticGroup;

    auto CreateFilePathEndsWithSeparatorError(
        const std::filesystem::path& path
    ) -> DiagnosticGroup;
}
