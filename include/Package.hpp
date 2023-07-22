#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "SrcBuffer.hpp"
#include "FileBuffer.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class Compilation;

    struct Package
    {
        static auto Parse(
            std::vector<std::shared_ptr<const ISrcBuffer>>* const srcBuffers,
            const FileBuffer* const fileBuffer
        ) -> Expected<Package>;

        std::string Name{};
        std::vector<const FileBuffer*> SrcFileBuffers{};
        std::vector<std::filesystem::path> DepFilePaths{};
    };
}
