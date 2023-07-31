#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "String.hpp"
#include "FileBuffer.hpp"

namespace Ace
{
    auto CreateJsonError(
        const FileBuffer* const fileBuffer,
        const nlohmann::json::exception& jsonException
    ) -> std::shared_ptr<const Diagnostic>;
}
