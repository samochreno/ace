#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "SourceBuffer.hpp"
#include "FileBuffer.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class Compilation;

    struct Package
    {
        static auto Parse(
            std::vector<std::shared_ptr<const ISourceBuffer>>* const sourceBuffers,
            const FileBuffer* const fileBuffer
        ) -> Expected<Package>;

        std::string Name{};
        std::vector<const FileBuffer*> SourceFileBuffers{};
        std::vector<std::filesystem::path> DependencyFilePaths{};
    };
}
