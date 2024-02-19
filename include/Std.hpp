#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Compilation.hpp"
#include "FileBuffer.hpp"

namespace Ace::Std
{
    auto GetName() -> const std::string&;

    auto CreateFileBuffers(
        Compilation* const compilation
    ) -> std::vector<std::shared_ptr<const FileBuffer>>;
}
