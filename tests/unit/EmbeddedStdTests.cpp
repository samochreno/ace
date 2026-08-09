#include <array>
#include <filesystem>
#include <string>

#include "FileBuffer.hpp"
#include "Std.hpp"

int main()
{
    const std::array expectedPaths{
        std::filesystem::path{ "std/bool.ace" }, std::filesystem::path{ "std/float.ace" },
        std::filesystem::path{ "std/int.ace" },  std::filesystem::path{ "std/mem.ace" },
        std::filesystem::path{ "std/op.ace" },   std::filesystem::path{ "std/print.ace" },
        std::filesystem::path{ "std/ptr.ace" },  std::filesystem::path{ "std/rc.ace" },
        std::filesystem::path{ "std/ref.ace" },  std::filesystem::path{ "std/string.ace" },
    };

    const auto fileBuffers = Ace::Std::CreateFileBuffers(nullptr);
    if (fileBuffers.size() != expectedPaths.size())
    {
        return 1;
    }

    for (size_t i = 0; i < fileBuffers.size(); ++i)
    {
        const auto& fileBuffer = fileBuffers.at(i);
        if (fileBuffer->GetPath() != expectedPaths.at(i) || fileBuffer->GetBuffer().empty())
        {
            return 1;
        }

        const auto expectedLocation = expectedPaths.at(i).string() + ":1:1";
        if (fileBuffer->FormatLocation(fileBuffer->CreateFirstLocation()) != expectedLocation)
        {
            return 1;
        }
    }

    return 0;
}
