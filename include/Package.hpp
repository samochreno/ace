#pragma once

#include <string>
#include <filesystem>
#include <vector>

#include "Error.hpp"

namespace Ace
{
    struct Package
    {
        static auto New(
            const std::filesystem::path& t_filePath
        ) -> Expected<Package>;

        std::string Name{};
        std::vector<std::filesystem::path> FilePaths{};
        std::vector<std::filesystem::path> DependencyFilePaths{};
    };
}
