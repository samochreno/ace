#pragma once

#include <memory>
#include <cstddef>
#include <filesystem>

namespace Ace
{
    class Compilation;

    struct SourceLocation
    {
        SourceLocation() = default;
        SourceLocation(
            const Compilation* const t_compilation,
            const std::shared_ptr<const std::filesystem::path>& t_filePath,
            const size_t& t_lineIndex,
            const std::string::const_iterator& t_itBegin,
            const std::string::const_iterator& t_itEnd
        ) : Compilation{ t_compilation },
            FilePath{ t_filePath },
            LineIndex{ t_lineIndex },
            IteratorBegin{ t_itBegin },
            IteratorEnd{ t_itEnd }
        {
        }

        const Compilation* Compilation{};
        std::shared_ptr<const std::filesystem::path> FilePath{};
        size_t LineIndex{};
        std::string::const_iterator IteratorBegin{};
        std::string::const_iterator IteratorEnd{};
    };
}
