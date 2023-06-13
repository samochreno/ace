#pragma once

#include <string>

namespace Ace
{
    class File;

    struct SourceLocation
    {
        SourceLocation() = default;
        SourceLocation(
            const File* const t_file,
            const size_t& t_lineIndex,
            const std::string::const_iterator& t_itBegin,
            const std::string::const_iterator& t_itEnd
        ) : File{ t_file },
            LineIndex{ t_lineIndex },
            IteratorBegin{ t_itBegin },
            IteratorEnd{ t_itEnd }
        {
        }

        const File* File{};
        size_t LineIndex{};
        std::string::const_iterator IteratorBegin{};
        std::string::const_iterator IteratorEnd{};
    };
}
