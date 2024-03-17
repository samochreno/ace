#pragma once

#include <memory>
#include <string_view>
#include "Assert.hpp"

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
        );
        SrcLocation(
            const SrcLocation& first,
            const SrcLocation& last
        );

        auto CreateFirst() const -> SrcLocation;
        auto CreateLast()  const -> SrcLocation;

        static auto CreateInterstice(
            const SrcLocation& first,
            const SrcLocation& last
        ) -> SrcLocation;

        const ISrcBuffer* Buffer{};
        std::string_view::const_iterator CharacterBeginIterator{};
        std::string_view::const_iterator CharacterEndIterator{};
    };
}
