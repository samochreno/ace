#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <vector>

#include "Diagnostics.hpp"

namespace Ace
{
    struct Package
    {
        static auto New(
            const std::filesystem::path& t_filePath
        ) -> Expected<Package>;

        std::string Name{};
        std::vector<std::shared_ptr<const std::filesystem::path>> FilePaths{};
        std::vector<std::shared_ptr<const std::filesystem::path>> DependencyFilePaths{};
    };
}
