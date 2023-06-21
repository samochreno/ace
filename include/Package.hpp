#pragma once

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
            const FileBuffer* const t_fileBuffer
        ) -> Expected<Package>;

        std::string Name{};
        std::vector<FileBuffer> SourceFileBuffers{};
        std::vector<std::filesystem::path> DependencyFilePaths{};
    };
}
