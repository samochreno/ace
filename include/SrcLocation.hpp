#pragma once

#include <memory>
#include <string_view>

namespace Ace
{
    class Compilation;
    class ISrcBuffer;

    struct SrcLocation
    {
        SrcLocation() = default;
        SrcLocation(Compilation* const compilation);
        SrcLocation(
            const ISrcBuffer* const buffer,
            const std::string_view::const_iterator characterBeginIt,
            const std::string_view::const_iterator characterEndIt
        ) : Buffer{ buffer },
            CharacterBeginIterator{ characterBeginIt },
            CharacterEndIterator{ characterEndIt }
        {
        }
        SrcLocation(
            const SrcLocation& first,
            const SrcLocation& last
        ) : Buffer{ first.Buffer },
            CharacterBeginIterator{ first.CharacterBeginIterator },
            CharacterEndIterator{ last.CharacterEndIterator }
        {
        }

        auto CreateFirst() const -> SrcLocation
        {
            return
            {
                Buffer,
                CharacterBeginIterator,
                CharacterBeginIterator + 1,
            };
        }
        auto CreateLast() const -> SrcLocation
        {
            return
            {
                Buffer,
                CharacterEndIterator - 1,
                CharacterEndIterator,
            };
        }

        const ISrcBuffer* Buffer{};
        std::string_view::const_iterator CharacterBeginIterator{};
        std::string_view::const_iterator CharacterEndIterator{};
    };
}
