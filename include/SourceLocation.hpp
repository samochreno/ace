#pragma once

#include <vector>
#include <string>

namespace Ace
{
    class File;

    struct SourceLocation
    {
        SourceLocation() = default;
        SourceLocation(
            const File* const t_file,
            const std::vector<std::string>::const_iterator t_lineIt,
            const std::string::const_iterator& t_characterItBegin,
            const std::string::const_iterator& t_characterItEnd
        ) : File{ t_file },
            LineIterator{ t_lineIt },
            CharacterIteratorBegin{ t_characterItBegin },
            CharacterIteratorEnd{ t_characterItEnd }
        {
        }

        const File* File{};
        std::vector<std::string>::const_iterator LineIterator{};
        std::string::const_iterator CharacterIteratorBegin{};
        std::string::const_iterator CharacterIteratorEnd{};
    };
}
