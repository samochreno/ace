#pragma once

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Token.hpp"
#include "Nodes/All.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto ParseAST(
        const FileBuffer* const t_fileBuffer,
        const std::vector<std::shared_ptr<const Token>>& t_tokens
    ) -> Expected<std::shared_ptr<const ModuleNode>>;
}
