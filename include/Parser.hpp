#pragma once

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "Token.hpp"
#include "Nodes/All.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto ParseAST(
        const FileBuffer* const fileBuffer,
        const std::vector<std::shared_ptr<const Token>>& tokens
    ) -> Expected<std::shared_ptr<const ModuleNode>>;
}
