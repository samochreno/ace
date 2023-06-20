#pragma once

#include <string_view>

#include "SourceLocation.hpp"

namespace Ace
{
    class Compilation;

    class ISourceBuffer
    {
    public:
        virtual ~ISourceBuffer() = default;

        virtual auto GetCompilation() const -> const Compilation* = 0;
        virtual auto GetBuffer() const -> std::string_view = 0;

        virtual auto FormatLocation(
            const SourceLocation& t_location
        ) const -> std::string = 0;
    };
}
