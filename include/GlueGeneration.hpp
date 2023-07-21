#pragma once

#include <memory>

#include "Compilation.hpp"
#include "Emittable.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"

namespace Ace::GlueGeneration
{
    auto GenerateAndBindGlue(
        const Compilation* const compilation
    ) -> void;

    auto CreateCopyGlueBody(
        const Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;
    auto CreateDropGlueBody(
        const Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;
}
