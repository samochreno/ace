#pragma once

#include <cstddef>
#include <filesystem>

namespace Ace
{
    class Compilation;

    struct SourceLocation
    {
        SourceLocation(
            const Compilation* const t_compilation,
            const size_t& t_fileIndex,
            const size_t& t_line,
            const size_t& t_character,
            const size_t& t_length
        ) : Compilation{ t_compilation },
            FileIndex{ t_fileIndex },
            Line{ t_line },
            Character{ t_character },
            Length{ t_length }
        {
        }

        const Compilation* Compilation{};
        size_t FileIndex{};
        size_t Line{};
        size_t Character{};
        size_t Length{};
    };
}
