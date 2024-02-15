#pragma once

#include <memory>
#include <vector>

#include "FileBuffer.hpp"
#include "Token.hpp"
#include "Diagnostic.hpp"
#include "Syntaxes/All.hpp"

namespace Ace
{
    auto ParseAST(
        const FileBuffer* const fileBuffer
    ) -> Expected<std::shared_ptr<const ModSyntax>>;
}
