#include "SrcLocation.hpp"

#include <string_view>

#include "Compilation.hpp"

namespace Ace
{
    SrcLocation::SrcLocation(
        Compilation* const compilation
    ) : Buffer{ compilation->GetCLIArgBuffer() },
        CharacterBeginIterator{ begin(std::string_view{ compilation->GetCLIArgBuffer()->GetBuffer() }) },
        CharacterEndIterator  { end  (std::string_view{ compilation->GetCLIArgBuffer()->GetBuffer() }) }
    {
    }

    SrcLocation::SrcLocation(
        const ISrcBuffer* const buffer,
        const std::string_view::const_iterator characterBeginIt,
        const std::string_view::const_iterator characterEndIt
    ) : Buffer{ buffer },
        CharacterBeginIterator{ characterBeginIt },
        CharacterEndIterator{ characterEndIt }
    {
        ACE_ASSERT(characterBeginIt <= characterEndIt);
    }
    SrcLocation::SrcLocation(
        const SrcLocation& first,
        const SrcLocation& last
    ) : Buffer{ first.Buffer },
        CharacterBeginIterator{ first.CharacterBeginIterator },
        CharacterEndIterator{ last.CharacterEndIterator }
    {
        ACE_ASSERT(first.Buffer == last.Buffer);
        ACE_ASSERT(first.CharacterBeginIterator <= last.CharacterEndIterator);
    }

    auto SrcLocation::CreateFirst() const -> SrcLocation
    {
        return
        {
            Buffer,
            CharacterBeginIterator,
            CharacterBeginIterator + 1,
        };
    }

    auto SrcLocation::CreateLast() const -> SrcLocation
    {
        return
        {
            Buffer,
            CharacterEndIterator - 1,
            CharacterEndIterator,
        };
    }

    auto SrcLocation::CreateInterstice(
        const SrcLocation& first,
        const SrcLocation& last
    ) -> SrcLocation
    {
        ACE_ASSERT(first.Buffer == last.Buffer);

        if (first.CharacterEndIterator == last.CharacterBeginIterator)
        {
            return first.CreateLast();
        }

        ACE_ASSERT(first.CharacterEndIterator < last.CharacterBeginIterator);

        return
        {
            first.Buffer,
            first.CharacterEndIterator,
            last.CharacterBeginIterator,
        };
    }
}
