#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "Diagnostics.hpp"

namespace Ace
{
    class Compilation;

    struct File
    {
        static auto New(
            const Compilation* const t_compilation,
            const std::filesystem::path& t_path
        ) -> Expected<File>;

        const Compilation* Compilation{};
        std::filesystem::path Path{};
        std::vector<std::string> Lines{};
    };
}
