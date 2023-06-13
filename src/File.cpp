#include "File.hpp"

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

#include "Compilation.hpp"

namespace Ace
{
    auto File::New(
        const Ace::Compilation* const t_compilation,
        const std::filesystem::path& t_path
    ) -> Expected<File>
    {
        std::ifstream fileStream{ t_path };
        ACE_TRY_ASSERT(fileStream.is_open());

        std::vector<std::string> lines{};
        std::string line{};
        while (std::getline(fileStream, line))
        {
            lines.push_back(line);
        }

        return File{ t_compilation, t_path, lines };
    }
}
