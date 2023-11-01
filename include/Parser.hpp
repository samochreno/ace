#pragma once

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "Token.hpp"
#include "Syntaxes/All.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto ParseAST(
        const FileBuffer* const fileBuffer,
        std::vector<Token> tokens
    ) -> Expected<std::shared_ptr<const ModSyntax>>;
}
