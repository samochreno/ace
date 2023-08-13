#pragma once

#include <memory>
#include <vector>

#include "Token.hpp"
#include "Diagnostic.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto LexTokens(
        const FileBuffer* const fileBuffer
    ) -> Diagnosed<std::vector<Token>>;
}
