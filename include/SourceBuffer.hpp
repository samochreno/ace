#pragma once

#include <string>
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
        virtual auto GetBuffer() const -> const std::string& = 0;

        virtual auto FormatLocation(
            const SourceLocation& location
        ) const -> std::string = 0;
    };
}
