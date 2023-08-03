#pragma once

#include <string>
#include <string_view>

#include "SrcLocation.hpp"

namespace Ace
{
    class Compilation;

    class ISrcBuffer
    {
    public:
        virtual ~ISrcBuffer() = default;

        virtual auto GetCompilation() const -> Compilation* = 0;
        virtual auto GetBuffer() const -> const std::string& = 0;

        virtual auto FormatLocation(
            const SrcLocation& location
        ) const -> std::string = 0;
    };
}
