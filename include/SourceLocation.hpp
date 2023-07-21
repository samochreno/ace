#pragma once

#include <string_view>

namespace Ace
{
    class ISourceBuffer;

    struct SourceLocation
    {
        SourceLocation() = default;
        SourceLocation(
            const ISourceBuffer* const buffer,
            const std::string_view::const_iterator characterBeginIt,
            const std::string_view::const_iterator characterEndIt
        ) : Buffer{ buffer },
            CharacterBeginIterator{ characterBeginIt },
            CharacterEndIterator{ characterEndIt }
        {
        }
        SourceLocation(
            const SourceLocation& first,
            const SourceLocation& last
        ) : Buffer{ first.Buffer },
            CharacterBeginIterator{ first.CharacterBeginIterator },
            CharacterEndIterator{ last.CharacterEndIterator }
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
