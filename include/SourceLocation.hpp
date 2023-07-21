#pragma once

#include <string_view>

namespace Ace
{
    class ISourceBuffer;

    struct SourceLocation
    {
        SourceLocation() = default;
        SourceLocation(
            const ISourceBuffer* const t_buffer,
            const std::string_view::const_iterator t_characterBeginIt,
            const std::string_view::const_iterator t_characterEndIt
        ) : Buffer{ t_buffer },
            CharacterBeginIterator{ t_characterBeginIt },
            CharacterEndIterator{ t_characterEndIt }
        {
        }
        SourceLocation(
            const SourceLocation& t_first,
            const SourceLocation& t_last
        ) : Buffer{ t_first.Buffer },
            CharacterBeginIterator{ t_first.CharacterBeginIterator },
            CharacterEndIterator{ t_last.CharacterEndIterator }
        {
        }

        auto CreateFirst() const -> SourceLocation
        {
            return
            {
                Buffer,
                CharacterBeginIterator,
                CharacterBeginIterator + 1,
            };
        }
        auto CreateLast() const -> SourceLocation
        {
            return
            {
                Buffer,
                CharacterEndIterator - 1,
                CharacterEndIterator,
            };
        }

        const ISourceBuffer* Buffer{};
        std::string_view::const_iterator CharacterBeginIterator{};
        std::string_view::const_iterator CharacterEndIterator{};
    };
}
