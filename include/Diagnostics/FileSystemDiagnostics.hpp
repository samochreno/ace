#pragma once

#include <memory>
#include <filesystem>

#include "Diagnostic.hpp"

namespace Ace
{
    auto CreateFileSystemError(
        const std::filesystem::path& path,
        const std::filesystem::filesystem_error& error
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateFileNotFoundError(
        const std::filesystem::path& path
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateFileOpenError(
        const std::filesystem::path& path
    ) -> std::shared_ptr<const Diagnostic>;

    auto CreateFilePathEndsWithSeparatorError(
        const std::filesystem::path& path
    ) -> std::shared_ptr<const Diagnostic>;
}
