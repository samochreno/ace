#pragma once

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
            const size_t& t_fileIndex,
            const size_t& t_lineIndex,
            const std::string::const_iterator& t_itBegin,
            const std::string::const_iterator& t_itEnd
        ) : Compilation{ t_compilation },
            FileIndex{ t_fileIndex },
            LineIndex{ t_lineIndex },
            IteratorBegin{ t_itBegin },
            IteratorEnd{ t_itEnd }
        {
        }

        const Compilation* Compilation{};
        size_t FileIndex{};
        size_t LineIndex{};
        std::string::const_iterator IteratorBegin{};
        std::string::const_iterator IteratorEnd{};
    };
}
