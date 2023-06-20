#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <vector>

#include "Diagnostics.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    class Compilation;

    struct Package
    {
        static auto New(
            const Compilation* const t_compilation,
            const std::filesystem::path& t_filePath
        ) -> Expected<Package>;

        std::string Name{};
        std::vector<FileBuffer> SourceFileBuffers{};
        std::vector<std::filesystem::path> DependencyFilePaths{};
    };
}
